#include "printing_selector_card_selection_widget.h"

/**
 * @brief Constructs a PrintingSelectorCardSelectionWidget for navigating through cards in the deck.
 *
 * This widget provides buttons that allow users to navigate between cards in the deck.
 * It includes buttons for moving to the previous and next card in the deck.
 *
 * @param parent The parent PrintingSelector widget responsible for managing card selection.
 */
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

/**
 * @brief Connects the signals from the buttons to the appropriate slots in the parent widget.
 *
 * This method connects the click signals of the previous and next card buttons to
 * the selectPreviousCard and selectNextCard slots in the parent PrintingSelector widget.
 */
void PrintingSelectorCardSelectionWidget::connectSignals()
{
    connect(previousCardButton, &QPushButton::clicked, parent, &PrintingSelector::selectPreviousCard);
    connect(nextCardButton, &QPushButton::clicked, parent, &PrintingSelector::selectNextCard);
}
