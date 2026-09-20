#include "visual_deck_storage_sort_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "visual_deck_storage_widget.h"

#include <libcockatrice/settings/visual_deck_storage_settings.h>

VisualDeckStorageSortWidget::VisualDeckStorageSortWidget(VisualDeckStorageWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // Initialize the ComboBox
    sortComboBox = new QComboBox(this);
    layout->addWidget(sortComboBox);

    // Need to retranslateUi first so that the sortComboBox actually has entries and doesn't get its currentIndex
    // immediately capped to 0 when we try to set it
    retranslateUi();

    // Set the current sort order
    sortComboBox->setCurrentIndex(SettingsCache::instance().visualDeckStorage().getVisualDeckStorageSortingOrder());

    // Connect sorting change signal to persist the order and refresh the file list
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VisualDeckStorageSortWidget::updateSortOrder);
}

void VisualDeckStorageSortWidget::retranslateUi()
{
    // Block signals to avoid triggering unnecessary updates while changing text
    sortComboBox->blockSignals(true);

    int oldIndex = sortComboBox->currentIndex();

    // Clear and repopulate the ComboBox with translated items
    sortComboBox->clear();
    sortComboBox->addItem(tr("Sort Alphabetically (Deck Name)"),
                          VisualDeckStorageSortFilterProxyModel::SortOrder::ByName);
    sortComboBox->addItem(tr("Sort Alphabetically (Filename)"),
                          VisualDeckStorageSortFilterProxyModel::SortOrder::Alphabetical);
    sortComboBox->addItem(tr("Sort by Last Modified"),
                          VisualDeckStorageSortFilterProxyModel::SortOrder::ByLastModified);
    sortComboBox->addItem(tr("Sort by Last Loaded"), VisualDeckStorageSortFilterProxyModel::SortOrder::ByLastLoaded);

    // Restore the current index
    sortComboBox->setCurrentIndex(oldIndex);

    // Re-enable signals
    sortComboBox->blockSignals(false);
}

VisualDeckStorageSortFilterProxyModel::SortOrder VisualDeckStorageSortWidget::currentSortOrder() const
{
    return static_cast<VisualDeckStorageSortFilterProxyModel::SortOrder>(sortComboBox->currentIndex());
}

void VisualDeckStorageSortWidget::updateSortOrder()
{
    SettingsCache::instance().visualDeckStorage().setVisualDeckStorageSortingOrder(sortComboBox->currentIndex());
    emit sortOrderChanged();
}
