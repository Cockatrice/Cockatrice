#include "visual_database_display_color_filter_widget.h"

#include "../cards/additional_info/mana_symbol_widget.h"

#include <QSet>
#include <QTimer>

VisualDatabaseDisplayColorFilterWidget::VisualDatabaseDisplayColorFilterWidget(QWidget *parent,
                                                                               FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel), layout(new QHBoxLayout(this))
{
    setLayout(layout);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);
    setMaximumHeight(50);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QString fullColorIdentity = "WUBRG";
    for (const QChar &color : fullColorIdentity) {
        auto *manaSymbol = new ManaSymbolWidget(this, color, false, true);
        manaSymbol->setMaximumHeight(25);
        manaSymbol->setMaximumWidth(25);

        layout->addWidget(manaSymbol);

        // Initialize the activeColors map
        activeColors[color] = false;

        // Connect the color toggled signal
        connect(manaSymbol, &ManaSymbolWidget::colorToggled, this,
                &VisualDatabaseDisplayColorFilterWidget::handleColorToggled);
    }

    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);

    // Connect the button's toggled signal
    connect(toggleButton, &QPushButton::toggled, this, &VisualDatabaseDisplayColorFilterWidget::updateFilterMode);
    connect(this, &VisualDatabaseDisplayColorFilterWidget::activeColorsChanged, this,
            &VisualDatabaseDisplayColorFilterWidget::updateColorFilter);
    connect(this, &VisualDatabaseDisplayColorFilterWidget::filterModeChanged, this,
            &VisualDatabaseDisplayColorFilterWidget::updateColorFilter);
    connect(filterModel, &FilterTreeModel::layoutChanged, this, [this]() {
        if (blockSync) {
            return; // Skip sync if we're blocking it
        }
        QTimer::singleShot(100, this, &VisualDatabaseDisplayColorFilterWidget::syncWithFilterModel);
    });

    // Call retranslateUi to set the initial text
    retranslateUi();
}

void VisualDatabaseDisplayColorFilterWidget::retranslateUi()
{
    switch (currentMode) {
        case FilterMode::ExactMatch:
            toggleButton->setText(tr("Mode: Exact Match"));
            break;
        case FilterMode::Includes:
            toggleButton->setText(tr("Mode: Includes"));
            break;
        case FilterMode::IncludeExclude:
            toggleButton->setText(tr("Mode: Include/Exclude"));
            break;
    }
}

void VisualDatabaseDisplayColorFilterWidget::handleColorToggled(QChar color, bool active)
{
    switch (currentMode) {
        case FilterMode::ExactMatch:
            // In Exact Match Mode, only allow the selected colors
            if (active) {
                activeColors[color] = true; // Add color to the selected list
            } else {
                activeColors[color] = false; // Remove color from the selected list
            }
            break;

        case FilterMode::Includes:
            // In Includes Mode, toggle the color freely (include selected colors)
            activeColors[color] = active;
            break;

        case FilterMode::IncludeExclude:
            // In Include/Exclude Mode, toggle the color freely (include selected, exclude unselected)
            if (active) {
                activeColors[color] = true; // Include the color
            } else {
                activeColors[color] = false; // Exclude the color
            }
            break;
    }

    emit activeColorsChanged(); // Notify listeners that the active colors have changed
}

void VisualDatabaseDisplayColorFilterWidget::updateColorFilter()
{
    blockSync = true;

    // Clear previous filters
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrColor);

    QSet<QString> selectedColors;
    QSet<QString> excludedColors;

    // Collect active colors in the selected and excluded sets
    for (const auto &color : activeColors.keys()) {
        if (activeColors[color]) {
            selectedColors.insert(color); // Include this color
        } else {
            excludedColors.insert(color); // Exclude this color
        }
    }

    switch (currentMode) {
        case FilterMode::ExactMatch:
            // Exact Match Mode: Only selected colors are allowed
            if (!selectedColors.isEmpty()) {
                // Require all selected colors (TypeAnd)
                for (const auto &color : selectedColors) {
                    QString colorString = color;
                    filterModel->addFilter(
                        new CardFilter(colorString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor));
                }

                // Exclude all other colors
                QStringList allPossibleColors = {"W", "U", "B", "R", "G"};
                for (const auto &color : allPossibleColors) {
                    if (!selectedColors.contains(color)) {
                        QString colorString = color;
                        filterModel->addFilter(
                            new CardFilter(colorString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
                    }
                }
            }
            break;

        case FilterMode::Includes:
            // Includes Mode: Just include selected colors without restrictions
            for (const auto &color : selectedColors) {
                QString colorString = color;
                filterModel->addFilter(new CardFilter(colorString, CardFilter::Type::TypeOr,
                                                      CardFilter::Attr::AttrColor)); // OR for selected colors
            }
            break;

        case FilterMode::IncludeExclude:
            // Include/Exclude Mode: Include selected colors and exclude unselected colors
            for (const auto &color : selectedColors) {
                QString colorString = color;
                filterModel->addFilter(new CardFilter(colorString, CardFilter::Type::TypeOr,
                                                      CardFilter::Attr::AttrColor)); // OR for selected colors
            }
            for (const auto &color : excludedColors) {
                QString colorString = color;
                filterModel->addFilter(new CardFilter(colorString, CardFilter::Type::TypeAndNot,
                                                      CardFilter::Attr::AttrColor)); // AND NOT for excluded colors
            }
            break;
    }

    blockSync = false;
}

void VisualDatabaseDisplayColorFilterWidget::updateFilterMode(bool checked)
{
    blockSync = true;

    if (checked) {
        // Cycle through the modes in a clear manner
        switch (currentMode) {
            case FilterMode::ExactMatch:
                currentMode = FilterMode::Includes; // Switch to Includes
                break;
            case FilterMode::Includes:
                currentMode = FilterMode::IncludeExclude; // Switch to Include/Exclude
                break;
            case FilterMode::IncludeExclude:
                currentMode = FilterMode::ExactMatch; // Switch to Exact Match
                break;
        }
    }

    retranslateUi();                     // Update button text based on the mode
    emit filterModeChanged(currentMode); // Signal mode change
    updateColorFilter();                 // Reapply the filter based on the new mode

    blockSync = false;
}

void VisualDatabaseDisplayColorFilterWidget::syncWithFilterModel()
{
    blockSync = true;
    QSet<QString> currentFilters;

    // Get current filters of type color
    for (const auto &filter : filterModel->getFiltersOfType(CardFilter::Attr::AttrColor)) {
        if (filter->type() == CardFilter::Type::TypeAnd || filter->type() == CardFilter::Type::TypeOr) {
            currentFilters.insert(filter->term());
        }
    }

    QSet<QString> activeFilterList;

    // Iterate over the activeColors map and collect the active colors as strings
    for (auto it = activeColors.constBegin(); it != activeColors.constEnd(); ++it) {
        if (it.value()) { // Only add active colors
            activeFilterList.insert(QString(it.key()));
        }
    }

    // Check if the filters in the model match the active filters
    if (currentFilters == activeFilterList) {
        return;
    }

    // Remove filters that are in the UI but not in the model
    for (const auto &color : activeFilterList) {
        if (!currentFilters.contains(color)) {
            activeColors[color[0]] = false; // Disable the color
        }
    }

    // Add filters that are in the model but not in the UI
    for (const auto &color : currentFilters) {
        if (!activeFilterList.contains(color)) {
            activeColors[color[0]] = true; // Enable the color
        }
    }

    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        manaSymbolWidget->setColorActive(activeColors[manaSymbolWidget->getSymbolChar()]);
    }

    updateColorFilter();
    blockSync = false;
}