#include "visual_deck_storage_sort_widget.h"

#include "../../../../settings/cache_settings.h"

/**
 * @brief Constructs a PrintingSelectorCardSortWidget for searching cards by set name or set code.
 *
 * This widget provides a search bar that allows users to search for cards by either their set name
 * or set code. It uses a debounced timer to trigger the search action after the user stops typing.
 *
 * @param parent The parent PrintingSelector widget that will handle the search results.
 */
VisualDeckStorageSortWidget::VisualDeckStorageSortWidget(VisualDeckStorageWidget *parent)
    : parent(parent), sortOrder(Alphabetical)
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
    sortComboBox->setCurrentIndex(SettingsCache::instance().getVisualDeckStorageSortingOrder());
    sortOrder = static_cast<SortOrder>(sortComboBox->currentIndex());

    // Connect sorting change signal to refresh the file list
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VisualDeckStorageSortWidget::updateSortOrder);
    connect(this, &VisualDeckStorageSortWidget::sortOrderChanged, parent, &VisualDeckStorageWidget::updateSortOrder);
}

void VisualDeckStorageSortWidget::retranslateUi()
{
    // Block signals to avoid triggering unnecessary updates while changing text
    sortComboBox->blockSignals(true);

    int oldIndex = sortComboBox->currentIndex();

    // Clear and repopulate the ComboBox with translated items
    sortComboBox->clear();
    sortComboBox->addItem(tr("Sort Alphabetically (Deck Name)"), ByName);
    sortComboBox->addItem(tr("Sort Alphabetically (Filename)"), Alphabetical);
    sortComboBox->addItem(tr("Sort by Last Modified"), ByLastModified);
    sortComboBox->addItem(tr("Sort by Last Loaded"), ByLastLoaded);

    // Restore the current index
    sortComboBox->setCurrentIndex(oldIndex);

    // Re-enable signals
    sortComboBox->blockSignals(false);
}

void VisualDeckStorageSortWidget::updateSortOrder()
{
    sortOrder = static_cast<SortOrder>(sortComboBox->currentIndex());
    SettingsCache::instance().setVisualDeckStorageSortingOrder(sortComboBox->currentIndex());
    emit sortOrderChanged();
}

/**
 * Gets the comparator that is used for the current sortOrder
 */
std::function<bool(DeckPreviewWidget *, DeckPreviewWidget *)> VisualDeckStorageSortWidget::getComparator() const
{
    // Sort the widgets list based on the current sort order
    return [sortOrder = sortOrder](const DeckPreviewWidget *widget1, const DeckPreviewWidget *widget2) {
        if (!widget1 || !widget2) {
            return false; // Handle null pointers gracefully
        }

        QFileInfo info1(widget1->filePath);
        QFileInfo info2(widget2->filePath);

        switch (sortOrder) {
            case ByName:
                return widget1->deckLoader->getName() < widget2->deckLoader->getName();
            case Alphabetical:
                return QString::localeAwareCompare(info1.fileName(), info2.fileName()) <= 0;
            case ByLastModified:
                return info1.lastModified() > info2.lastModified();
            case ByLastLoaded: {
                QDateTime time1 = QDateTime::fromString(widget1->deckLoader->getLastLoadedTimestamp());
                QDateTime time2 = QDateTime::fromString(widget2->deckLoader->getLastLoadedTimestamp());
                return time1 > time2;
            }
        }

        return false; // Default case, no sorting applied
    };
}