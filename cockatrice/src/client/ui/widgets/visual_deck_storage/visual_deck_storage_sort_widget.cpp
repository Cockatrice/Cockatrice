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
    setLayout(layout);

    // ComboBox for sorting options
    sortComboBox = new QComboBox(this);
    sortComboBox->addItem("Sort Alphabetically (Deck Name)", ByName);
    sortComboBox->addItem("Sort Alphabetically (Filename)", Alphabetical);
    sortComboBox->addItem("Sort by Last Modified", ByLastModified);
    sortComboBox->addItem("Sort by Last Loaded", ByLastLoaded);
    sortComboBox->setCurrentIndex(SettingsCache::instance().getVisualDeckStorageSortingOrder());

    // Connect sorting change signal to refresh the file list
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VisualDeckStorageSortWidget::updateSortOrder);
    connect(this, &VisualDeckStorageSortWidget::sortOrderChanged, parent, &VisualDeckStorageWidget::updateSortOrder);

    layout->addWidget(sortComboBox);
}

void VisualDeckStorageSortWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    sortComboBox->setCurrentIndex(SettingsCache::instance().getVisualDeckStorageSortingOrder());
    updateSortOrder();
}

void VisualDeckStorageSortWidget::updateSortOrder()
{
    sortOrder = static_cast<SortOrder>(sortComboBox->currentIndex());
    qDebug() << "Sort order updated to: " << sortOrder;
    SettingsCache::instance().setVisualDeckStorageSortingOrder(sortComboBox->currentIndex());
    emit sortOrderChanged();
}

QList<DeckPreviewWidget *> &VisualDeckStorageSortWidget::filterFiles(QList<DeckPreviewWidget *> &widgets)
{
    // Sort the widgets list based on the current sort order
    std::sort(widgets.begin(), widgets.end(), [this](DeckPreviewWidget *widget1, DeckPreviewWidget *widget2) {
        if (!widget1 || !widget2) {
            return false; // Handle null pointers gracefully
        }

        QFileInfo info1(widget1->filePath);
        QFileInfo info2(widget2->filePath);

        switch (sortOrder) {
            case ByName:
                return widget1->deckLoader->getName() < widget2->deckLoader->getName();
            case Alphabetical:
                return info1.fileName().toLower() < info2.fileName().toLower();
            case ByLastModified:
                return info1.lastModified() > info2.lastModified();
            case ByLastLoaded:
                return widget1->deckLoader->getLastLoadedTimestamp() > widget2->deckLoader->getLastLoadedTimestamp();
        }

        return false; // Default case, no sorting applied
    });

    return widgets;
}
