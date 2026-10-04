#include "card_database_parser.h"

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/database/card_database_data.h"

#include <QSharedPointer>

class ICardSetPriorityController;

SetNameMap ICardDatabaseParser::sets;

ICardDatabaseParser::ICardDatabaseParser(ICardSetPriorityController *_cardSetPriorityController)
    : cardSetPriorityController(_cardSetPriorityController)
{
}
void ICardDatabaseParser::clearSetlist()
{
    sets.clear();
}

CardSetPtr ICardDatabaseParser::internalAddSet(const QString &setName,
                                               const QString &longName,
                                               const QString &setType,
                                               const QDate &releaseDate,
                                               const CardSet::Priority priority)
{
    if (sets.contains(setName)) {
        return sets.value(setName);
    }

    CardSetPtr newSet = CardSet::newInstance(cardSetPriorityController, setName);
    newSet->setLongName(longName);
    newSet->setSetType(setType);
    newSet->setReleaseDate(releaseDate);
    newSet->setPriority(priority);

    sets.insert(setName, newSet);
    if (targetData) {
        targetData->sets.insert(setName, newSet);
    } else {
        emit addSet(newSet);
    }
    return newSet;
}
