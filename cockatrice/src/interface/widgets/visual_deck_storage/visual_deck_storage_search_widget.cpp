#include "visual_deck_storage_search_widget.h"

#include "../../../filters/syntax_help.h"
#include "../../pixel_map_generator.h"
#include "visual_deck_storage_widget.h"

#include <QAction>
#include <QTimer>

/**
 * @brief Constructs a search bar for filtering decks in the Visual Deck Storage.
 *
 * Provides a search bar that allows users to search decks by filename or search
 * expression, with a debounced timer to trigger the search after the user stops typing.
 *
 * @param parent The VisualDeckStorageWidget owning the search.
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

    connect(searchDebounceTimer, &QTimer::timeout, this,
            [this] { this->parent->proxyModel()->setSearchText(searchBar->text()); });
}
