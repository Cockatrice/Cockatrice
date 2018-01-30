#ifndef HANDCOUNTER_H
#define HANDCOUNTER_H

#include "abstractgraphicsitem.h"
#include <QString>

class QPainter;
class QPixmap;

class HandCounter : public AbstractGraphicsItem
{
    Q_OBJECT
private:
    int number;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
public slots:
    void updateNumber();
signals:
    void showContextMenu(const QPoint &screenPos);

public:
    enum
    {
        Type = typeOther
    };
    int type() const
    {
        return Type;
    }
    HandCounter(QGraphicsItem *parent = 0);
    ~HandCounter();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
