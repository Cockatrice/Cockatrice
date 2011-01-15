#ifndef ZONEVIEWERZONE_H
#define ZONEVIEWERZONE_H

#include "selectzone.h"
#include <QGraphicsLayoutItem>

class ZoneViewWidget;
class ProtocolResponse;
class ServerInfo_Card;

class ZoneViewZone : public SelectZone, public QGraphicsLayoutItem {
	Q_OBJECT
private:
	QRectF bRect, optimumRect;
	int minRows, numberCards;
	void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
	CardZone *origZone;
	bool revealZone;
	bool sortByName, sortByType;
public:
	ZoneViewZone(Player *_p, CardZone *_origZone, int _numberCards = -1, bool _revealZone = false, QGraphicsItem *parent = 0);
	~ZoneViewZone();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void initializeCards(const QList<ServerInfo_Card *> &cardList = QList<ServerInfo_Card *>());
	void removeCard(int position);
	int getNumberCards() const { return numberCards; }
	void setGeometry(const QRectF &rect);
	QRectF getOptimumRect() const { return optimumRect; }
public slots:
	void setSortByName(int _sortByName);
	void setSortByType(int _sortByType);
private slots:
	void zoneDumpReceived(ProtocolResponse *r);
signals:
	void beingDeleted();
	void optimumRectChanged();
protected:
	void addCardImpl(CardItem *card, int x, int y);
	QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
};

#endif
