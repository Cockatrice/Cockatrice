#include "visual_database_display_color_filter_widget.h"

#include "../cards/additional_info/essence_filter_picker.h"
#include "filter_tree_model.h"
#include "libcockatrice/filters/filter_card.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QSharedPointer>
#include <QString>
#include <QVariant>
#include <libcockatrice/card/essence_types.h>
#include <libcockatrice/filters/filter_tree.h>

VisualDatabaseDisplayColorFilterWidget::VisualDatabaseDisplayColorFilterWidget(QWidget *parent,
                                                                               FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel), layout(new QHBoxLayout(this))
{
    setLayout(layout);
    layout->setSpacing(3);
    layout->setContentsMargins(0, 0, 0, 0);

    // Share the same compact Essence selection UI as Visual Deck Storage.
    // A 15-icon horizontal row cannot fit the database filter toolbar.
    essencePicker = new EssenceFilterPicker(this);
    layout->addWidget(essencePicker);

    connect(essencePicker, &EssenceFilterPicker::essenceToggled, this,
            &VisualDatabaseDisplayColorFilterWidget::handleColorToggled);

    modeComboBox = new QComboBox(this);
    layout->addWidget(modeComboBox);

    connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VisualDatabaseDisplayColorFilterWidget::updateFilterMode);

    connect(filterModel, &FilterTreeModel::layoutChanged, this,
            [this]() { syncDebouncer.call([this] { syncWithFilterModel(); }); });

    retranslateUi();
    syncWithFilterModel();
}

void VisualDatabaseDisplayColorFilterWidget::retranslateUi()
{
    essencePicker->retranslateUi();

    modeComboBox->blockSignals(true);
    modeComboBox->clear();
    modeComboBox->addItem(tr("Exact match"), QVariant::fromValue(FilterMode::ExactMatch));
    modeComboBox->addItem(tr("Includes"), QVariant::fromValue(FilterMode::Includes));
    modeComboBox->addItem(tr("Include / Exclude"), QVariant::fromValue(FilterMode::IncludeExclude));
    modeComboBox->setToolTip(tr("How selected and unselected Essence types are combined in the filter"));

    const int index = modeComboBox->findData(QVariant::fromValue(currentMode));
    if (index >= 0) {
        modeComboBox->setCurrentIndex(index);
    }
    modeComboBox->blockSignals(false);
}

void VisualDatabaseDisplayColorFilterWidget::handleColorToggled(QChar essence, bool active)
{
    if (active) {
        addFilter(essence);
    } else {
        removeFilter(essence);
    }
}

void VisualDatabaseDisplayColorFilterWidget::addFilter(QChar essence)
{
    QString essenceString(essence);
    const QList<const CardFilter *> filters = filterModel->getFiltersOfType(CardFilter::Attr::AttrColor);
    for (const CardFilter *filter : filters) {
        if (filter->term() == essenceString) {
            filterModel->removeFilter(filter);
        }
    }

    switch (currentMode) {
        case FilterMode::ExactMatch:
            filterModel->addFilter(
                new CardFilter(essenceString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor));
            break;
        case FilterMode::Includes:
        case FilterMode::IncludeExclude:
            filterModel->addFilter(
                new CardFilter(essenceString, CardFilter::Type::TypeOr, CardFilter::Attr::AttrColor));
            break;
    }
}

void VisualDatabaseDisplayColorFilterWidget::removeFilter(QChar essence)
{
    QString essenceString(essence);
    const QList<const CardFilter *> filters = filterModel->getFiltersOfType(CardFilter::Attr::AttrColor);
    for (const CardFilter *filter : filters) {
        if (filter->term() == essenceString) {
            filterModel->removeFilter(filter);
        }
    }

    switch (currentMode) {
        case FilterMode::ExactMatch:
        case FilterMode::IncludeExclude:
            filterModel->addFilter(
                new CardFilter(essenceString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
            break;
        case FilterMode::Includes:
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

    const QSet<QChar> selected = essencePicker->activeEssences();
    for (const QChar essence : Essence::symbols()) {
        handleColorToggled(essence, selected.contains(essence));
    }

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
    emit filterModeChanged(currentMode);
}

void VisualDatabaseDisplayColorFilterWidget::setEssenceActive(QChar essence, bool active)
{
    essencePicker->setEssenceActive(essence, active);
}

QList<QChar> VisualDatabaseDisplayColorFilterWidget::getActiveColors()
{
    QList<QChar> result;
    const QSet<QChar> selected = essencePicker->activeEssences();
    for (const QChar essence : Essence::symbols()) {
        if (selected.contains(essence)) {
            result.append(essence);
        }
    }
    return result;
}

void VisualDatabaseDisplayColorFilterWidget::syncWithFilterModel()
{
    const QList<const CardFilter *> filters = filterModel->getFiltersOfType(CardFilter::Attr::AttrColor);

    for (const QChar essence : Essence::symbols()) {
        bool selected = false;
        for (const CardFilter *filter : filters) {
            if (filter->term() != QString(essence)) {
                continue;
            }
            if (currentMode == FilterMode::ExactMatch) {
                selected = filter->type() == CardFilter::Type::TypeAnd;
            } else {
                selected = filter->type() == CardFilter::Type::TypeOr;
            }
            if (selected) {
                break;
            }
        }
        // Signals are blocked inside the picker; loading saved filters must
        // not mutate the FilterTreeModel again.
        setEssenceActive(essence, selected);
    }
}
