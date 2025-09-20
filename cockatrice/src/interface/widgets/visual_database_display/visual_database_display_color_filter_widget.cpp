#include "visual_database_display_color_filter_widget.h"

#include "../../../filters/filter_tree.h"
#include "../cards/additional_info/mana_symbol_widget.h"

#include <QSet>
#include <QTimer>

/**
 * This widget provides a graphical control element for the CardFilter::Attr::AttrColor filters applied to the filter
 * model.
 * @param parent The Qt Widget that this widget will parent to
 * @param _filterModel The filter model that this widget will manipulate
 */
VisualDatabaseDisplayColorFilterWidget::VisualDatabaseDisplayColorFilterWidget(QWidget *parent,
                                                                               FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel), layout(new QHBoxLayout(this))
{
    setLayout(layout);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);

    QString fullColorIdentity = "WUBRG";
    for (const QChar &color : fullColorIdentity) {
        auto *manaSymbol = new ManaSymbolWidget(this, color, false, true);
        manaSymbol->setFixedWidth(25);

        layout->addWidget(manaSymbol);

        // Connect the color toggled signal
        connect(manaSymbol, &ManaSymbolWidget::colorToggled, this,
                &VisualDatabaseDisplayColorFilterWidget::handleColorToggled);
    }

    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);

    // Connect the button's toggled signal
    connect(toggleButton, &QPushButton::toggled, this, &VisualDatabaseDisplayColorFilterWidget::updateFilterMode);
    connect(filterModel, &FilterTreeModel::layoutChanged, this,
            [this]() { QTimer::singleShot(100, this, &VisualDatabaseDisplayColorFilterWidget::syncWithFilterModel); });

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

    toggleButton->setToolTip(tr("Filter mode (AND/OR/NOT conjunctions of filters)"));
}

void VisualDatabaseDisplayColorFilterWidget::handleColorToggled(QChar color, bool active)
{
    if (active) {
        addFilter(color);
    } else {
        removeFilter(color);
    }
}

void VisualDatabaseDisplayColorFilterWidget::addFilter(QChar color)
{
    QString colorString = color;
    QString typeStr;

    // Remove previous filters

    QList<const CardFilter *> allColorFilters = filterModel->getFiltersOfType(CardFilter::Attr::AttrColor);
    QList<const CardFilter *> matchingFilters;

    for (const CardFilter *filter : allColorFilters) {
        if (filter->term() == color) {
            matchingFilters.append(filter);
        }
    }

    for (const CardFilter *filter : matchingFilters) {
        filterModel->removeFilter(filter);
    }

    // Add actual filter

    switch (currentMode) {
        case FilterMode::ExactMatch:
            filterModel->addFilter(new CardFilter(colorString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::Includes:
            filterModel->addFilter(new CardFilter(colorString, CardFilter::Type::TypeOr, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::IncludeExclude:
            filterModel->addFilter(new CardFilter(colorString, CardFilter::Type::TypeOr, CardFilter::Attr::AttrColor));
            break;
    }
}

void VisualDatabaseDisplayColorFilterWidget::removeFilter(QChar color)
{
    QString colorString = color;

    // Remove inclusion filters
    QList<const CardFilter *> allColorFilters = filterModel->getFiltersOfType(CardFilter::Attr::AttrColor);
    QList<const CardFilter *> matchingFilters;

    for (const CardFilter *filter : allColorFilters) {
        if (filter->term() == color) {
            matchingFilters.append(filter);
        }
    }

    for (const CardFilter *filter : matchingFilters) {
        filterModel->removeFilter(filter);
    }

    // Add exclusion filters if the mode demands it
    switch (currentMode) {
        case FilterMode::ExactMatch:
            filterModel->addFilter(
                new CardFilter(colorString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::IncludeExclude:
            filterModel->addFilter(
                new CardFilter(colorString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::Includes:
            // No exclusion in Includes mode
            break;
    }
}

void VisualDatabaseDisplayColorFilterWidget::updateFilterMode()
{
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

    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);

    filterModel->clearFiltersOfType(CardFilter::Attr::AttrColor);

    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        handleColorToggled(manaSymbolWidget->getSymbolChar(), manaSymbolWidget->isColorActive());
    }

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();

    retranslateUi();                     // Update button text based on the mode
    emit filterModeChanged(currentMode); // Signal mode change
}

void VisualDatabaseDisplayColorFilterWidget::setManaSymbolActive(QChar color, bool active)
{
    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        if (manaSymbolWidget->getSymbolChar() == color) {
            manaSymbolWidget->setColorActive(active);
        }
    }
}

QList<QChar> VisualDatabaseDisplayColorFilterWidget::getActiveColors()
{
    QList<QChar> activeColors;
    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        if (manaSymbolWidget->isColorActive()) {
            activeColors.append(manaSymbolWidget->getSymbolChar());
        }
    }

    return activeColors;
}

void VisualDatabaseDisplayColorFilterWidget::syncWithFilterModel()
{
    QList<const CardFilter *> allColorFilters = filterModel->getFiltersOfType(CardFilter::Attr::AttrColor);

    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        bool found = false;
        for (const CardFilter *filter : allColorFilters) {
            if (manaSymbolWidget->getSymbolChar() == filter->term()) {
                switch (currentMode) {
                    case FilterMode::ExactMatch:
                        switch (filter->type()) {
                            case CardFilter::Type::TypeAnd:
                                setManaSymbolActive(filter->term().at(0), true);
                                break;
                            default:
                                setManaSymbolActive(filter->term().at(0), false);
                                break;
                        }
                        break;
                    case FilterMode::Includes:
                        switch (filter->type()) {
                            case CardFilter::Type::TypeOr:
                                setManaSymbolActive(filter->term().at(0), true);
                                break;
                            default:
                                setManaSymbolActive(filter->term().at(0), false);
                                break;
                        }
                        break;
                    case FilterMode::IncludeExclude:
                        switch (filter->type()) {
                            case CardFilter::Type::TypeOr:
                                setManaSymbolActive(filter->term().at(0), true);
                                break;
                            default:
                                setManaSymbolActive(filter->term().at(0), false);
                                break;
                        }
                        break;
                }
                found = true;
            }
        }

        if (!found) {
            setManaSymbolActive(manaSymbolWidget->getSymbolChar(), false);
        }
    }
}
