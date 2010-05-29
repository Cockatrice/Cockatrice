#ifndef HANDZONE_H
#define HANDZONE_H

#include "cardzone.h"

class HandZone : public CardZone {
	Q_OBJECT
protected:
	QPixmap bgPixmap;
private slots:
	void updateBgPixmap();
public:
	HandZone(Player *_p, bool _contentsKnown, QGraphicsItem *parent = 0);
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
	virtual void setWidth(qreal _width) = 0;
protected:
	void addCardImpl(CardItem *card, int x, int y);
};

#endif

