#include "carddatabaseparser.h"

SetNameMap ICardDatabaseParser::sets;

void ICardDatabaseParser::clearSetlist()
{
    sets.clear();
}

CardSetPtr ICardDatabaseParser::internalAddSet(const QString &setName,
                                               const QString &longName,
                                               const QString &setType,
                                               const QDate &releaseDate)
{
    if (sets.contains(setName)) {
        return sets.value(setName);
    }

    CardSetPtr newSet = CardSet::newInstance(setName);
    newSet->setLongName(longName);
    newSet->setSetType(setType);
    newSet->setReleaseDate(releaseDate);

    sets.insert(setName, newSet);
    emit addSet(newSet);
    return newSet;
}