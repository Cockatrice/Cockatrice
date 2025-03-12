#include "visual_database_display_color_filter_widget.h"

#include "../cards/additional_info/mana_symbol_widget.h"

#include <QSet>

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

    // Call retranslateUi to set the initial text
    retranslateUi();
}

void VisualDatabaseDisplayColorFilterWidget::retranslateUi()
{
    // Set the toggle button text based on the current mode
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
}

void VisualDatabaseDisplayColorFilterWidget::handleColorToggled(QChar color, bool active)
{
    activeColors[color] = active;
    emit activeColorsChanged();
}

void VisualDatabaseDisplayColorFilterWidget::updateColorFilter()
{
    // Clear existing filters related to color
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrColor);

    if (exactMatchMode) {
        // Exact match: card must have ONLY the selected colors
        QSet<QString> selectedColors;
        for (const auto &color : activeColors.keys()) {
            if (activeColors[color]) {
                selectedColors.insert(color);
            }
        }

        if (!selectedColors.isEmpty()) {
            // Require all selected colors (TypeAnd)
            for (const auto &color : selectedColors) {
                QString colorString = color;
                filterModel->addFilter(
                    new CardFilter(colorString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor));
            }

            // Exclude any other colors (TypeAndNot)
            QStringList allPossibleColors = {"W", "U", "B", "R", "G"};
            for (const auto &color : allPossibleColors) {
                if (!selectedColors.contains(color)) {
                    QString colorString = color;
                    filterModel->addFilter(
                        new CardFilter(colorString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
                }
            }
        }
    } else {
        // Default includes mode (TypeAnd) - match any selected colors
        for (const auto &color : activeColors.keys()) {
            if (activeColors[color]) { // If the color is active
                QString colorString = color;
                filterModel->addFilter(
                    new CardFilter(colorString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor));
            }
        }
    }
}

void VisualDatabaseDisplayColorFilterWidget::updateFilterMode(bool checked)
{
    exactMatchMode = checked; // Toggle between modes
    retranslateUi();          // Update the button text
    emit filterModeChanged(exactMatchMode);
}
