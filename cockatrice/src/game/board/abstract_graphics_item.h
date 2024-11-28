#ifndef ABSTRACTGRAPHICSITEM_H
#define ABSTRACTGRAPHICSITEM_H

#include <QGraphicsItem>

enum GraphicsItemType
{
    typeCard = QGraphicsItem::UserType + 1,
    typeCardDrag = QGraphicsItem::UserType + 2,
    typeZone = QGraphicsItem::UserType + 3,
    typePlayerTarget = QGraphicsItem::UserType + 4,
    typeDeckViewCardContainer = QGraphicsItem::UserType + 5,
    typeOther = QGraphicsItem::UserType + 6
};

/**
 * Parent class of all objects that appear in a game.
 */
class AbstractGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
protected:
    void paintNumberEllipse(int number, int radius, const QColor &color, int position, int count, QPainter *painter);

public:
    AbstractGraphicsItem(QGraphicsItem *parent = nullptr) : QObject(), QGraphicsItem(parent)
    {
    }
};

int resetPainterTransform(QPainter *painter);

#endif
