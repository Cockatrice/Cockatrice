#ifndef TABLEZONE_H
#define TABLEZONE_H

#include "cardzone.h"

class TableZone : public QObject, public CardZone {
	Q_OBJECT
signals:
	void sizeChanged();
private:
	int width, height;
	QPixmap bgPixmap;
	bool economicGrid;
public:
	static const int paddingY = 20;
	static const int marginX = 20;
	static const int minWidth = 20;

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
	QPointF closestGridPoint(const QPointF &point);
	CardItem *takeCard(int position, int cardId, const QString &cardName);
protected:
	void addCardImpl(CardItem *card, int x, int y);
};

#endif
