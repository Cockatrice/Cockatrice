#include "visual_deck_storage_search_widget.h"

#include "../../../../settings/cache_settings.h"

/**
 * @brief Constructs a PrintingSelectorCardSearchWidget for searching cards by set name or set code.
 *
 * This widget provides a search bar that allows users to search for cards by either their set name
 * or set code. It uses a debounced timer to trigger the search action after the user stops typing.
 *
 * @param parent The parent PrintingSelector widget that will handle the search results.
 */
VisualDeckStorageSearchWidget::VisualDeckStorageSearchWidget(VisualDeckStorageWidget *parent) : parent(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText(tr("Search by filename"));
    layout->addWidget(searchBar);

    // Add a debounce timer for the search bar to limit frequent updates
    searchDebounceTimer = new QTimer(this);
    searchDebounceTimer->setSingleShot(true);
    connect(searchBar, &QLineEdit::textChanged, this, [this]() {
        searchDebounceTimer->start(300); // 300ms debounce
    });

    connect(searchDebounceTimer, &QTimer::timeout, parent, &VisualDeckStorageWidget::updateSearchFilter);
}

/**
 * @brief Retrieves the current text in the search bar.
 *
 * @return The text entered by the user in the search bar.
 */
QString VisualDeckStorageSearchWidget::getSearchText()
{
    return searchBar->text();
}

/**
 * Gets the filename used for the search.
 *
 * if includeFolderName is true, then this returns the relative filepath starting from the deck folder.
 * If the file isn't in the deck folder, or includeFolderName is false, then this will just return the filename.
 *
 * @param filePath The filePath to convert into a search name
 */
static QString getFileSearchName(const QString &filePath, bool includeFolderName)
{
    QString deckPath = SettingsCache::instance().getDeckPath();
    if (includeFolderName && filePath.startsWith(deckPath)) {
        return filePath.mid(deckPath.length()).toLower();
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName().toLower();
    return fileName;
}

void VisualDeckStorageSearchWidget::filterWidgets(QList<DeckPreviewWidget *> widgets,
                                                  const QString &searchText,
                                                  bool includeFolderName)
{
    if (searchText.isEmpty() || searchText.isNull()) {
        for (auto widget : widgets) {
            widget->filteredBySearch = false;
        }
    }

    for (auto file : widgets) {
        QString fileSearchName = getFileSearchName(file->filePath, includeFolderName);
        file->filteredBySearch = !fileSearchName.contains(searchText.toLower());
    }
}
