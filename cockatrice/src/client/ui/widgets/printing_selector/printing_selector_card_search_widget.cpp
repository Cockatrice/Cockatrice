#include "printing_selector_card_search_widget.h"

PrintingSelectorCardSearchWidget::PrintingSelectorCardSearchWidget(PrintingSelector *parent) : parent(parent)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText(tr("Search by set name or set code"));
    layout->addWidget(searchBar);

    // Add a debounce timer for the search bar
    searchDebounceTimer = new QTimer(this);
    searchDebounceTimer->setSingleShot(true);
    connect(searchBar, &QLineEdit::textChanged, this, [this]() {
        searchDebounceTimer->start(300); // 300ms debounce
    });

    connect(searchDebounceTimer, &QTimer::timeout, parent, &PrintingSelector::updateDisplay);
}

QString PrintingSelectorCardSearchWidget::getSearchText()
{
    return searchBar->text();
}