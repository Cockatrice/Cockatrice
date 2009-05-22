#ifndef ZONEVIEWERZONE_H
#define ZONEVIEWERZONE_H

#include "cardzone.h"
#include "serverzonecard.h"

class ZoneViewWidget;

class ZoneViewZone : public CardZone {
private:
	int height;
	int numberCards;
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
	CardZone *origZone;
signals:
	void removeZoneViewWidget(ZoneViewWidget *zv);
public:
	ZoneViewZone(Player *_p, CardZone *_origZone, int _numberCards = 0, QGraphicsItem *parent = 0);
	~ZoneViewZone();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	bool initializeCards();
	void removeCard(int position);
	void setHeight(int _height) { height = _height; }
protected:
	void addCardImpl(CardItem *card, int x, int y);
};

#endif
