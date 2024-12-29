#include "visual_deck_storage_search_widget.h"

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

    connect(searchDebounceTimer, &QTimer::timeout, parent, &VisualDeckStorageWidget::refreshBannerCards);
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

QStringList VisualDeckStorageSearchWidget::filterFiles(const QStringList &files, const QString &searchText)
{
    if (searchText.isEmpty() || searchText.isNull()) {
        return files;
    }

    QStringList filteredFiles;

    for (const auto &file : files) {
        QFileInfo fileInfo(file);
        QString fileName = fileInfo.fileName().toLower();

        if (fileName.contains(searchText.toLower())) {
            filteredFiles << file;
        }
    }

    return filteredFiles;
}
