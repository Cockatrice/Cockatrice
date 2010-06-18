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
	typeOther = QGraphicsItem::UserType + 5
};

class AbstractCardItem : public QObject, public AbstractGraphicsItem {
	Q_OBJECT
protected:
	CardInfo *info;
	QString name;
	bool tapped;
	QString color;
private slots:
	void pixmapUpdated();
signals:
	void hovered(AbstractCardItem *card);
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
	QString getColor() const { return color; }
	void setColor(const QString &_color);
	bool getTapped() const { return tapped; }
	void setTapped(bool _tapped);
	void processHoverEvent();
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
};
 
#endif
