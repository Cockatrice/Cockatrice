#ifndef COCKATRICE_COMMANDER_SPELLBOOK_DECK_REQUEST_H
#define COCKATRICE_COMMANDER_SPELLBOOK_DECK_REQUEST_H
#include "card_in_deck_request.h"
#include "libcockatrice/deck_list/deck_list.h"

#include <QJsonObject>
#include <QVector>

class CommanderSpellbookDeckRequest
{
public:
    CommanderSpellbookDeckRequest() = default;

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
    void fromDeckList(const DeckList &deck);

    void debugPrint() const;

    const QVector<CardInDeckRequest> &main() const
    {
        return mainDeck;
    }
    const QVector<CardInDeckRequest> &commanders() const
    {
        return commanderDeck;
    }

private:
    QVector<CardInDeckRequest> mainDeck;      // maxItems: 600
    QVector<CardInDeckRequest> commanderDeck; // maxItems: 12
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_DECK_REQUEST_H
