#ifndef COCKATRICE_COMMANDER_SPELLBOOK_DECK_REQUEST_H
#define COCKATRICE_COMMANDER_SPELLBOOK_DECK_REQUEST_H
#include "card_in_deck_request.h"

#include <QJsonObject>
#include <QList>

class DeckList;

struct CommanderSpellbookDeckRequest
{
    static CommanderSpellbookDeckRequest fromJson(const QJsonObject &json);
    static CommanderSpellbookDeckRequest fromDeckList(const DeckList &deck);
    QJsonObject toJson() const;

    QList<CardInDeckRequest> mainDeck;      // maxItems: 600
    QList<CardInDeckRequest> commanderDeck; // maxItems: 12
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_DECK_REQUEST_H
