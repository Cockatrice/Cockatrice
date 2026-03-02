/**
 * @file abstract_graphics_item.h
 * @ingroup GameGraphics
 * @brief TODO: Document this.
 */

#ifndef ABSTRACTGRAPHICSITEM_H
#define ABSTRACTGRAPHICSITEM_H

#include <QGraphicsItem>

/**
 * Parent class of all objects that appear in a game.
 */
class AbstractGraphicsItem : public QGraphicsObject
{
    Q_OBJECT

protected:
    void paintNumberEllipse(int number, int radius, const QColor &color, int position, int count, QPainter *painter);

public:
    explicit AbstractGraphicsItem(QGraphicsItem *parent = nullptr) : QGraphicsObject(parent)
    {
    }
};

int resetPainterTransform(QPainter *painter);

#endif
