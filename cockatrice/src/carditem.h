#ifndef CARDITEM_H
#define CARDITEM_H

#include "abstractcarditem.h"

class CardDatabase;
class CardDragItem;
class CardZone;
class ServerInfo_Card;
class Player;

const int MAX_COUNTERS_ON_CARD = 999;

class CardItem : public AbstractCardItem {
	Q_OBJECT
private:
	Player *owner;
	int id;
	bool attacking;
	bool facedown;
	int counters;
	QString annotation;
	bool doesntUntap;
	QPoint gridPoint;
	CardDragItem *dragItem;
public:
	enum { Type = typeCard };
	int type() const { return Type; }
	CardItem(Player *_owner, const QString &_name = QString(), int _cardid = -1, QGraphicsItem *parent = 0);
	~CardItem();
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QPoint getGridPoint() const { return gridPoint; }
	void setGridPoint(const QPoint &_gridPoint) { gridPoint = _gridPoint; }
	Player *getOwner() const { return owner; }
	int getId() const { return id; }
	void setId(int _id) { id = _id; }
	bool getAttacking() const { return attacking; }
	void setAttacking(bool _attacking);
	bool getFaceDown() const { return facedown; }
	void setFaceDown(bool _facedown);
	int getCounters() const { return counters; }
	void setCounters(int _counters);
	QString getAnnotation() const { return annotation; }
	void setAnnotation(const QString &_annotation);
	bool getDoesntUntap() const { return doesntUntap; }
	void setDoesntUntap(bool _doesntUntap);
	void resetState();
	void processCardInfo(ServerInfo_Card *info);

	CardDragItem *createDragItem(int _id, const QPointF &_pos, const QPointF &_scenePos, bool faceDown);
	void deleteDragItem();
	void processHoverEvent();
signals:
	void hovered(CardItem *card);
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
};

#endif
