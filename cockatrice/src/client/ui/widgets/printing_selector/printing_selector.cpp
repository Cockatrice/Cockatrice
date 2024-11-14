#include "printing_selector.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "printing_selector_card_display_widget.h"

#include <QHBoxLayout>

PrintingSelector::PrintingSelector(DeckListModel *deckModel, QTreeView *deckView, QWidget *parent)
    : QWidget(parent), deckModel(deckModel), deckView(deckView)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout = new QHBoxLayout();
    setLayout(layout);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);
}

void PrintingSelector::setCard(CardInfoPtr newCard)
{
    selectedCard = newCard;
    flowWidget->clearLayout();
    getAllSetsForCurrentCard();
}

CardInfoPerSet PrintingSelector::getSetForUUID(QString uuid)
{
    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();

    for (auto cardInfoPerSet : cardInfoPerSets) {
        if (cardInfoPerSet.getProperty("uuid") == uuid) {
            return cardInfoPerSet;
        }
    }

    return CardInfoPerSet();
}

void PrintingSelector::getAllSetsForCurrentCard()
{
    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();
    for (auto cardInfoPerSet : cardInfoPerSets) {
        PrintingSelectorCardDisplayWidget *cardDisplayWidget =
            new PrintingSelectorCardDisplayWidget(deckModel, deckView, selectedCard, cardInfoPerSet);
        flowWidget->addWidget(cardDisplayWidget);
    }
}
