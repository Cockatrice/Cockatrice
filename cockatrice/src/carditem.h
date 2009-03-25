#ifndef CARDITEM_H
#define CARDITEM_H

#include <QGraphicsItem>

class CardDatabase;
class CardDragItem;
class CardZone;


const int CARD_WIDTH = 72;
const int CARD_HEIGHT = 102;
const int RASTER_WIDTH = 36;
const int RASTER_HEIGHT = 34;

/*
const int CARD_WIDTH = 72;
const int CARD_HEIGHT = 108;
const int RASTER_WIDTH = 36;
const int RASTER_HEIGHT = 36;
*/

const int MAX_COUNTERS_ON_CARD = 999;

enum CardItemType {
	typeCard = QGraphicsItem::UserType + 1,
	typeCardDrag = QGraphicsItem::UserType + 2,
	typeZone = QGraphicsItem::UserType + 3,
	typeOther = QGraphicsItem::UserType + 4
};

class CardItem : public QGraphicsItem {
private:
	CardDatabase *db;
	QString name;
	int id;
	int width, height;
	QPixmap *image;
	bool tapped;
	bool attacking;
	bool facedown;
	int counters;
	QString annotation;
	CardDragItem *dragItem;
public:
	enum { Type = typeCard };
	int type() const { return Type; }
	CardItem(CardDatabase *_db, const QString &_name, int _cardid, QGraphicsItem *parent = 0);
	~CardItem();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	int getId() const { return id; }
	void setId(int _id) { id = _id; }
	QString getName() const { return name; }
	void setName(const QString &_name = QString());
	QPixmap *getImage() const { return image; }
	bool getTapped() const { return tapped; }
	void setTapped(bool _tapped);
	bool getAttacking() const { return attacking; }
	void setAttacking(bool _attacking);
	bool getFacedown() const { return facedown; }
	void setFacedown(bool _facedown);
	int getCounters() const { return counters; }
	void setCounters(int _counters);
	QString getAnnotation() const { return annotation; }
	void setAnnotation(const QString &_annotation);
	void resetState();

	CardDragItem *createDragItem(CardZone *startZone, int _id, const QPointF &_pos, const QPointF &_scenePos);
	void deleteDragItem();
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
};

#endif
