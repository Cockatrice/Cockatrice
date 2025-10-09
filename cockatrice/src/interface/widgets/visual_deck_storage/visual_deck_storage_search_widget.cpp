#include "visual_deck_storage_search_widget.h"

#include "../../../filters/deck_filter_string.h"
#include "../../../filters/syntax_help.h"
#include "../../pixel_map_generator.h"

#include <QAction>
#include <libcockatrice/settings/cache_settings.h>

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
    searchBar->setPlaceholderText(tr("Search by filename (or search expression)"));
    searchBar->setClearButtonEnabled(true);
    searchBar->addAction(loadColorAdjustedPixmap("theme:icons/search"), QLineEdit::LeadingPosition);

    auto help = searchBar->addAction(QPixmap("theme:icons/info"), QLineEdit::TrailingPosition);
    connect(help, &QAction::triggered, this, [this] { createDeckSearchSyntaxHelpWindow(searchBar); });

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
 * Converts the filepath into a relative filepath starting from the deck folder.
 * If the file isn't in the deck folder, then this will just return the filename.
 *
 * @param filePath The filepath to convert into a relative filepath
 */
static QString toRelativeFilepath(const QString &filePath)
{
    QString deckPath = SettingsCache::instance().getDeckPath();
    if (filePath.startsWith(deckPath)) {
        return filePath.mid(deckPath.length());
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    return fileName;
}

void VisualDeckStorageSearchWidget::filterWidgets(QList<DeckPreviewWidget *> widgets, const QString &searchText)
{
    auto filterString = DeckFilterString(searchText);

    for (auto widget : widgets) {
        QString relativeFilePath = toRelativeFilepath(widget->filePath);
        widget->filteredBySearch = !filterString.check(widget, {relativeFilePath});
    }
}
