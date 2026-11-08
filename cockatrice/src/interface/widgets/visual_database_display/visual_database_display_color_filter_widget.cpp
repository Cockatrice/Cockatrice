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

namespace
{

/** @brief Remove the filters that apply to the given color and attribute from the given model. */
static void removeFilters(FilterTreeModel *filterModel, QChar color, CardFilter::Attr attr)
{
    for (const CardFilter *filter : filterModel->getFiltersOfType(attr)) {
        if (filter->term() == color) {
            filterModel->removeFilter(filter);
        }
    }
}
} // namespace

/**
 * This widget provides a graphical control element for the CardFilter::Attr::AttrColor and
 * CardFilter::Attr::AttrColorIdentity filters applied to the filter model.
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

    modeComboBox->addItem(tr("Includes"), QVariant::fromValue(FilterMode::Includes));
    modeComboBox->addItem(tr("Excludes"), QVariant::fromValue(FilterMode::Excludes));
    modeComboBox->addItem(tr("Include / Exclude"), QVariant::fromValue(FilterMode::IncludeExclude));
    modeComboBox->addItem(tr("Exact match"), QVariant::fromValue(FilterMode::ExactMatch));
    modeComboBox->addItem(tr("Color identity"), QVariant::fromValue(FilterMode::ColorIdentity));

    modeComboBox->setItemData(0, tr("Cards containing any of the selected colors"), Qt::ToolTipRole);
    modeComboBox->setItemData(1, tr("Cards containing none of the selected colors"), Qt::ToolTipRole);
    modeComboBox->setItemData(2, tr("Cards containing any of the selected colors and none of the unselected colors"),
                              Qt::ToolTipRole);
    modeComboBox->setItemData(3, tr("Cards being exactly of the selected colors"), Qt::ToolTipRole);
    modeComboBox->setItemData(4, tr("Cards whose color identity fits within the selected colors"), Qt::ToolTipRole);

    if (currentMode == FilterMode::Custom) {
        addCustomFilterModeItem();
    }

    modeComboBox->setToolTip(tr("How selected and unselected colors are combined in the filter"));
    modeComboBox->setAccessibleName(tr("Color filter mode"));

    // Restore current mode
    const int index = modeComboBox->findData(QVariant::fromValue(currentMode));
    if (index >= 0) {
        modeComboBox->setCurrentIndex(index);
    }

    modeComboBox->blockSignals(false);
}

void VisualDatabaseDisplayColorFilterWidget::setFilterMode(FilterMode filterMode)
{
    int index = modeComboBox->findData(QVariant::fromValue(filterMode));

    if (index < 0 && filterMode == FilterMode::Custom) {
        index = modeComboBox->count();
        addCustomFilterModeItem();
    }

    if (index >= 0) {
        modeComboBox->setCurrentIndex(index);
    }
}

void VisualDatabaseDisplayColorFilterWidget::addCustomFilterModeItem()
{
    int customIndex = modeComboBox->count();
    modeComboBox->addItem(tr("Custom"), QVariant::fromValue(FilterMode::Custom));
    modeComboBox->setItemData(customIndex, tr("Cards matching an unknown combination of color filters"),
                              Qt::ToolTipRole);
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
    FilterModeInfo info = getFilterModeInfo(currentMode);

    removeFilters(filterModel, color, info.attr);

    if (info.includeType != CardFilter::Type::TypeEnd) {
        filterModel->addFilter(new CardFilter(color, info.includeType, info.attr));
    }
}

void VisualDatabaseDisplayColorFilterWidget::removeFilter(QChar color)
{
    FilterModeInfo info = getFilterModeInfo(currentMode);

    removeFilters(filterModel, color, info.attr);

    // Add exclusion filters if the mode demands it.
    if (info.excludeType != CardFilter::Type::TypeEnd) {
        filterModel->addFilter(new CardFilter(color, info.excludeType, info.attr));
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
    QList<const CardFilter *> filters;
    filters.append(filterModel->getFiltersOfType(CardFilter::Attr::AttrColor));
    filters.append(filterModel->getFiltersOfType(CardFilter::Attr::AttrColorIdentity));

    // Re-derive the mode from the filters in the model, so that a loaded filter set selects the mode that
    // produced it (or Custom when none could have).
    currentMode = deriveFilterMode(filters, currentMode);

    // Update the combobox. Add a Custom item only if we have a custom filter mode.
    int customIndex = modeComboBox->findData(QVariant::fromValue(FilterMode::Custom));
    modeComboBox->blockSignals(true);
    if (currentMode == FilterMode::Custom) {
        if (customIndex < 0) {
            customIndex = modeComboBox->count();
            addCustomFilterModeItem();
        }
        modeComboBox->setCurrentIndex(customIndex);
    } else {
        if (customIndex >= 0) {
            modeComboBox->removeItem(customIndex);
        }
        int index = modeComboBox->findData(QVariant::fromValue(currentMode));
        if (index >= 0) {
            modeComboBox->setCurrentIndex(index);
        }
    }
    modeComboBox->blockSignals(false);

    // Update the mana symbol widgets by looking for the current mode's include or exclude filters.
    const FilterModeInfo modeInfo = getFilterModeInfo(currentMode);

    const bool useExclude =
        modeInfo.includeType == CardFilter::Type::TypeEnd && modeInfo.excludeType != CardFilter::Type::TypeEnd;
    const CardFilter::Type filterType = useExclude ? modeInfo.excludeType : modeInfo.includeType;

    filters = filterModel->getFiltersOfType(modeInfo.attr);

    for (ManaSymbolWidget *manaSymbolWidget : findChildren<ManaSymbolWidget *>()) {
        bool found = false;
        for (const CardFilter *filter : filters) {
            if (filter->type() == filterType && manaSymbolWidget->getSymbolChar() == filter->term()) {
                found = true;
                break;
            }
        }

        manaSymbolWidget->setColorActive(useExclude ? !found : found);
        manaSymbolWidget->setDisabled(currentMode == FilterMode::Custom);
    }
}
