#include "printing_selector.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/cards/card_info_picture.h"
#include "printing_selector_card_display_widget.h"

#include <QHBoxLayout>

PrintingSelector::PrintingSelector(DeckListModel *deckModel, QTreeView *deckView, CardInfoPtr &selectedCard, QWidget *parent)
    : QWidget(parent), deckModel(deckModel), deckView(deckView), selectedCard(selectedCard)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qDebug() << selectedCard->getName();
    layout = new QHBoxLayout();
    setLayout(layout);
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
        PrintingSelectorCardDisplayWidget *cardDisplayWidget = new PrintingSelectorCardDisplayWidget(deckModel, deckView, selectedCard, cardInfoPerSet);
        layout->addWidget(cardDisplayWidget);
    }
}

void PrintingSelector::populateCards()
{
    qDebug() << "VisualDeckEditorWidget::populateCards Starting";
    if (!deckModel)
        return;
    DeckList *decklist = deckModel->getDeckList();
    if (!decklist)
        return;
    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return;

    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        if (!currentZone)
            continue;
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;
            for (int k = 0; k < currentCard->getNumber(); ++k) {
                CardInfoPerSet setForCard = getSetForUUID(currentCard->getCardUuid());
                qDebug() << setForCard.getProperties();
                CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(currentCard->getName());
                if (info) {
                    qDebug() << info->getName();
                } else {
                    qDebug() << "Card not found in database!";
                }
            }
        }
    }
}
