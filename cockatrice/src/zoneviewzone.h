#ifndef ZONEVIEWERZONE_H
#define ZONEVIEWERZONE_H

#include "playerzone.h"
#include "serverzonecard.h"

class ZoneViewWidget;

class ZoneViewZone : public PlayerZone {
private:
	int numberCards;
	void handleDropEvent(int cardId, PlayerZone *startZone, const QPoint &dropPoint);
	PlayerZone *origZone;
signals:
	void removeZoneViewWidget(ZoneViewWidget *zv);
public:
	ZoneViewZone(Player *_p, PlayerZone *_origZone, int _numberCards = 0, QGraphicsItem *parent = 0);
	~ZoneViewZone();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(CardItem *card, bool reorganize = true, int x = 0, int y = -1);
	void reorganizeCards();
	bool initializeCards();
	void removeCard(int position);
};

#endif
