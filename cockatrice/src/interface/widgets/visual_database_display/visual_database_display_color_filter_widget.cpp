#include "visual_database_display_color_filter_widget.h"

#include "../cards/additional_info/mana_symbol_widget.h"
#include "filter_tree_model.h"
#include "libcockatrice/filters/filter_card.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QPersistentModelIndex>
#include <QSharedPointer>
#include <QString>
#include <QVariant>
#include <QtGlobal>
#include <libcockatrice/filters/filter_tree.h>

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

    modeComboBox = new QComboBox(this);
    layout->addWidget(modeComboBox);

    connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VisualDatabaseDisplayColorFilterWidget::updateFilterMode);

    connect(filterModel, &FilterTreeModel::layoutChanged, this,
            [this]() { syncDebouncer.call([this] { syncWithFilterModel(); }); });

    // Call retranslateUi to set the initial text
    retranslateUi();
}

void VisualDatabaseDisplayColorFilterWidget::retranslateUi()
{
    modeComboBox->blockSignals(true);
    modeComboBox->clear();

    modeComboBox->addItem(tr("Exact match"), QVariant::fromValue(FilterMode::ExactMatch));
    modeComboBox->addItem(tr("Includes"), QVariant::fromValue(FilterMode::Includes));
    modeComboBox->addItem(tr("Excludes"), QVariant::fromValue(FilterMode::Excludes));
    modeComboBox->addItem(tr("Include / Exclude"), QVariant::fromValue(FilterMode::IncludeExclude));
    modeComboBox->addItem(tr("Commander"), QVariant::fromValue(FilterMode::Commander));

    modeComboBox->setToolTip(tr("How selected and unselected colors are combined in the filter"));

    // Restore current mode
    const int index = modeComboBox->findData(QVariant::fromValue(currentMode));
    if (index >= 0) {
        modeComboBox->setCurrentIndex(index);
    }

    modeComboBox->blockSignals(false);
}

void VisualDatabaseDisplayColorFilterWidget::handleColorToggled(QChar color, bool active)
{
    if (active) {
        addFilter(color);
    } else {
        removeFilter(color);
    }
}

static void removeFilters(FilterTreeModel *filterModel, QChar color, CardFilter::Attr attr)
{
    for (const CardFilter *filter : filterModel->getFiltersOfType(attr)) {
        if (filter->term() == color) {
            filterModel->removeFilter(filter);
        }
    }
}

void VisualDatabaseDisplayColorFilterWidget::addFilter(QChar color)
{
    switch (currentMode) {
        case FilterMode::ExactMatch:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            filterModel->addFilter(new CardFilter(color, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::Includes:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            filterModel->addFilter(new CardFilter(color, CardFilter::Type::TypeOr, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::Excludes:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            filterModel->addFilter(new CardFilter(color, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::IncludeExclude:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            filterModel->addFilter(new CardFilter(color, CardFilter::Type::TypeOr, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::Commander:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColorIdentity);
            break;
    }
}

void VisualDatabaseDisplayColorFilterWidget::removeFilter(QChar color)
{
    // Add exclusion filters if the mode demands it
    switch (currentMode) {
        case FilterMode::ExactMatch:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            filterModel->addFilter(new CardFilter(color, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::Includes:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::Excludes:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::IncludeExclude:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            filterModel->addFilter(new CardFilter(color, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
            break;

        case FilterMode::Commander:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColorIdentity);
            filterModel->addFilter(
                new CardFilter(color, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColorIdentity));
            break;
    }
}

void VisualDatabaseDisplayColorFilterWidget::updateFilterMode()
{
    const QVariant data = modeComboBox->currentData();
    if (!data.isValid()) {
        return;
    }

    currentMode = data.value<FilterMode>();

    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);

    filterModel->clearFiltersOfType(CardFilter::Attr::AttrColor);
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrColorIdentity);

    const QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        handleColorToggled(manaSymbolWidget->getSymbolChar(), manaSymbolWidget->isColorActive());
    }

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
    emit filterModeChanged(currentMode);
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
    QList<const CardFilter *> allColorFilters = filterModel->getFiltersOfType(
        currentMode == FilterMode::Commander ? CardFilter::Attr::AttrColorIdentity : CardFilter::Attr::AttrColor);

    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        QChar symbol = manaSymbolWidget->getSymbolChar();
        bool found = currentMode == FilterMode::Commander;
        for (const CardFilter *filter : allColorFilters) {
            if (symbol == filter->term()) {
                switch (currentMode) {
                    case FilterMode::ExactMatch:
                        if (filter->type() == CardFilter::Type::TypeAnd) {
                            found = true;
                        }
                        break;
                    case FilterMode::Includes:
                    case FilterMode::IncludeExclude:
                        if (filter->type() == CardFilter::Type::TypeOr) {
                            found = true;
                        }
                        break;
                    case FilterMode::Excludes:
                        if (filter->type() == CardFilter::Type::TypeAndNot) {
                            found = true;
                        }
                        break;
                    case FilterMode::Commander:
                        if (filter->type() == CardFilter::Type::TypeAndNot) {
                            found = false;
                        }
                        break;
                }
                if (found) {
                    break;
                }
            }
        }

        setManaSymbolActive(manaSymbolWidget->getSymbolChar(), found);
    }
}
