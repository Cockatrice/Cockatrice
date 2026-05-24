#ifndef SELECTION_SUBTYPE_COUNTER_H
#define SELECTION_SUBTYPE_COUNTER_H

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

namespace SelectionSubtypeCounter
{
QStringList extractSubtypesFromFace(const QString &faceType);
QList<MainTypeGroup> countSubtypes(const QList<CardItem *> &cards);
QString formatAsHtml(const QList<MainTypeGroup> &groups);
QString buildSubtypeCountText(const QList<CardItem *> &cards);
} // namespace SelectionSubtypeCounter

#endif
