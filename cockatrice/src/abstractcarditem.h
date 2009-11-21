#ifndef ABSTRACTCARDITEM_H
#define ABSTRACTCARDITEM_H

#include "abstractgraphicsitem.h"

class CardInfo;

const int CARD_WIDTH = 72;
const int CARD_HEIGHT = 102;

enum CardItemType {
	typeCard = QGraphicsItem::UserType + 1,
	typeCardDrag = QGraphicsItem::UserType + 2,
	typeZone = QGraphicsItem::UserType + 3,
	typeOther = QGraphicsItem::UserType + 4
};

class AbstractCardItem : public QObject, public AbstractGraphicsItem {
	Q_OBJECT
protected:
	CardInfo *info;
	QString name;
	bool tapped;
private slots:
	void pixmapUpdated();
public:
	enum { Type = typeCard };
	int type() const { return Type; }
	AbstractCardItem(const QString &_name = QString(), QGraphicsItem *parent = 0);
	~AbstractCardItem();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	CardInfo *getInfo() const { return info; }
	QString getName() const { return name; }
	void setName(const QString &_name = QString());
	bool getTapped() const { return tapped; }
	void setTapped(bool _tapped);
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
};
 
#endif
