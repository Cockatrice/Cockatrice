#ifndef TABLEZONE_H
#define TABLEZONE_H

#include "cardzone.h"

class TableZone : public CardZone {
private:
	int width, height;
	QPixmap bgPixmap;
public:
	static const int gridPointsPerCardX = 2;
	static const int gridPointsPerCardY = 1;
	static const int paddingY = 20;

	TableZone(Player *_p, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void toggleTapped();
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
	void handleDropEventByGrid(int cardId, CardZone *startZone, const QPoint &gridPoint, bool faceDown);
	CardItem *getCardFromGrid(const QPoint &gridPoint) const;
	QPointF mapFromGrid(const QPoint &gridPoint) const;
	QPoint mapToGrid(const QPointF &mapPoint) const;
	QPoint getFreeGridPoint(int row) const;
protected:
	void addCardImpl(CardItem *card, int x, int y);
};

#endif
