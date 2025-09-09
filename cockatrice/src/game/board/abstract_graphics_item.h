#ifndef ABSTRACTGRAPHICSITEM_H
#define ABSTRACTGRAPHICSITEM_H

#include <QGraphicsItem>

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
    explicit AbstractGraphicsItem(QGraphicsItem *parent = nullptr) : QGraphicsItem(parent)
    {
    }
};

int resetPainterTransform(QPainter *painter);

#endif
