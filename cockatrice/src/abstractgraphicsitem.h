#ifndef ABSTRACTGRAPHICSITEM_H
#define ABSTRACTGRAPHICSITEM_H

#include <QGraphicsItem>

class AbstractGraphicsItem : public QGraphicsItem {
protected:
	void paintNumberEllipse(int number, QPainter *painter);
public:
	AbstractGraphicsItem(QGraphicsItem *parent = 0) : QGraphicsItem(parent) { }
};

#endif
