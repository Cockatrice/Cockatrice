#include "printing_selector_card_selection_widget.h"

PrintingSelectorCardSelectionWidget::PrintingSelectorCardSelectionWidget(PrintingSelector *parent) : parent(parent)
{
    cardSelectionBarLayout = new QHBoxLayout(this);
    previousCardButton = new QPushButton(this);
    previousCardButton->setText(tr("Previous Card in Deck"));

    nextCardButton = new QPushButton(this);
    nextCardButton->setText(tr("Next Card in Deck"));

    connectSignals();

    cardSelectionBarLayout->addWidget(previousCardButton);
    cardSelectionBarLayout->addWidget(nextCardButton);
}

void PrintingSelectorCardSelectionWidget::connectSignals()
{
    connect(previousCardButton, &QPushButton::clicked, parent, &PrintingSelector::selectPreviousCard);
    connect(nextCardButton, &QPushButton::clicked, parent, &PrintingSelector::selectNextCard);
}