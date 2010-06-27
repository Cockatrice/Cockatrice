#ifndef ABSTRACTGRAPHICSITEM_H
#define ABSTRACTGRAPHICSITEM_H

#include <QGraphicsItem>

enum GraphicsItemType {
	typeCard = QGraphicsItem::UserType + 1,
	typeCardDrag = QGraphicsItem::UserType + 2,
	typeZone = QGraphicsItem::UserType + 3,
	typePlayerTarget = QGraphicsItem::UserType + 4,
	typeOther = QGraphicsItem::UserType + 5
};

class AbstractGraphicsItem : public QGraphicsItem {
protected:
	void paintNumberEllipse(int number, int radius, const QColor &color, int position, int count, QPainter *painter);
public:
	AbstractGraphicsItem(QGraphicsItem *parent = 0) : QGraphicsItem(parent) { }
};

#endif
