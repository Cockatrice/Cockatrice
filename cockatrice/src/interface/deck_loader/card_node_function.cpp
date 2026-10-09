#include "card_node_function.h"

#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/printing_info.h"
#include "libcockatrice/card/set/card_set.h"

#include <QSharedPointer>
#include <QString>
#include <QtGlobal>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>

void CardNodeFunction::SetProviderIdToPreferred::operator()(const InnerDecklistNode *node, DecklistCardNode *card) const
{
    Q_UNUSED(node);
    PrintingInfo preferredPrinting = CardDatabaseManager::query()->getPreferredPrinting(card->getName());
    CardSetPtr preferredSet = preferredPrinting.getSet();
    if (!preferredSet) {
        return;
    }

    card->setCardProviderId(preferredPrinting.getUuid());
    card->setCardCollectorNumber(preferredPrinting.getProperty("num"));
    card->setCardSetShortName(preferredSet->getShortName());
}

void CardNodeFunction::ClearPrintingData::operator()(const InnerDecklistNode *node, DecklistCardNode *card) const
{
    Q_UNUSED(node);
    card->setCardSetShortName(nullptr);
    card->setCardCollectorNumber(nullptr);
    card->setCardProviderId(nullptr);
}

void CardNodeFunction::ResolveProviderId::operator()(const InnerDecklistNode *node, DecklistCardNode *card) const
{
    Q_UNUSED(node);
    // Retrieve the providerId based on setName and collectorNumber
    QString providerId =
        CardDatabaseManager::getInstance()
            ->query()
            ->getSpecificPrinting(card->getName(), card->getCardSetShortName(), card->getCardCollectorNumber())
            .getUuid();

    // Set the providerId on the card
    card->setCardProviderId(providerId);
}