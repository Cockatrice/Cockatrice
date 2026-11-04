#include "deck_preview_color_identity_filter_widget.h"

#include "../../cards/additional_info/essence_filter_picker.h"
#include "../visual_deck_storage_sort_filter_proxy_model.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QSet>
#include <QString>

DeckPreviewColorIdentityFilterWidget::DeckPreviewColorIdentityFilterWidget(QWidget *parent)
    : QWidget(parent), layout(new QHBoxLayout(this))
{
    setLayout(layout);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);

    // Fifteen Essence codes don't fit as inline buttons beside sorting and
    // search. The popup keeps all codes selectable without expanding the row.
    essencePicker = new EssenceFilterPicker(this);
    layout->addWidget(essencePicker);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setMinimumWidth(essencePicker->width());

    connect(essencePicker, &EssenceFilterPicker::essenceToggled, this,
            [this](QChar /*essence*/, bool /*active*/) { emit activeColorsChanged(); });
    connect(essencePicker, &EssenceFilterPicker::selectionCleared, this,
            [this]() { emit activeColorsChanged(); });

    toggleButton = new QPushButton(this);
    essencePicker->addPopupControl(toggleButton);
    connect(toggleButton, &QPushButton::clicked, this, &DeckPreviewColorIdentityFilterWidget::updateFilterMode);

    retranslateUi();
}

void DeckPreviewColorIdentityFilterWidget::retranslateUi()
{
    essencePicker->retranslateUi();

    // Preserve existing Exact / Includes / Excludes filter semantics.
    switch (filterMode) {
        case VisualDeckStorageSortFilterProxyModel::FilterMode::ExactMatch:
            toggleButton->setText(tr("Mode: Exact Match"));
            break;
        case VisualDeckStorageSortFilterProxyModel::FilterMode::Includes:
            toggleButton->setText(tr("Mode: Includes"));
            break;
        case VisualDeckStorageSortFilterProxyModel::FilterMode::Excludes:
            toggleButton->setText(tr("Mode: Excludes"));
            break;
    }
    toggleButton->setToolTip(tr("Essence filter mode (exact match, includes, or excludes)"));
}

QSet<QChar> DeckPreviewColorIdentityFilterWidget::getActiveColors() const
{
    return essencePicker->activeEssences();
}

void DeckPreviewColorIdentityFilterWidget::handleColorToggled(QChar /*essence*/, bool /*active*/)
{
    emit activeColorsChanged();
}

void DeckPreviewColorIdentityFilterWidget::updateFilterMode()
{
    switch (filterMode) {
        case VisualDeckStorageSortFilterProxyModel::FilterMode::ExactMatch:
            filterMode = VisualDeckStorageSortFilterProxyModel::FilterMode::Includes;
            break;
        case VisualDeckStorageSortFilterProxyModel::FilterMode::Includes:
            filterMode = VisualDeckStorageSortFilterProxyModel::FilterMode::Excludes;
            break;
        case VisualDeckStorageSortFilterProxyModel::FilterMode::Excludes:
            filterMode = VisualDeckStorageSortFilterProxyModel::FilterMode::ExactMatch;
            break;
    }

    retranslateUi();
    emit filterModeChanged(filterMode);
}
