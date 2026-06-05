#ifndef SELECTION_SUBTYPE_TALLY_H
#define SELECTION_SUBTYPE_TALLY_H

#include <QList>
#include <QString>
#include <QStringList>

class CardItem;

struct SubtypeEntry
{
    QString name;
    int count;
};

struct MainTypeGroup
{
    QString mainType;
    int cardCount;
    QList<SubtypeEntry> subtypes;
};

namespace SelectionSubtypeTally
{
QList<MainTypeGroup> countSubtypes(const QList<CardItem *> &cards);
} // namespace SelectionSubtypeTally

#endif
