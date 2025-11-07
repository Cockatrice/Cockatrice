#include "card_database_parser.h"

#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>

SetNameMap ICardDatabaseParser::sets;

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

    CardSetPtr newSet = CardSet::newInstance(new NoopCardSetPriorityController(), setName);
    newSet->setLongName(longName);
    newSet->setSetType(setType);
    newSet->setReleaseDate(releaseDate);
    newSet->setPriority(priority);

    sets.insert(setName, newSet);
    emit addSet(newSet);
    return newSet;
}
