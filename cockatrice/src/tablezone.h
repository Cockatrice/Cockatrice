#ifndef TABLEZONE_H
#define TABLEZONE_H

#include "selectzone.h"

class TableZone : public SelectZone {
	Q_OBJECT
signals:
	void sizeChanged();
private:
	static const int boxLineWidth = 10;
	static const int paddingX = 35;
	static const int paddingY = 10;
	static const int marginX = 20;
	static const int minWidth = 15 * CARD_WIDTH / 2;

	QMap<int, int> gridPointWidth;
	int width, height;
	int currentMinimumWidth;
	QPixmap bgPixmap;
	bool active;
	bool isInverted() const;
private slots:	
	void updateBgPixmap();
public slots:
	void reorganizeCards();
public:
	TableZone(Player *_p, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void toggleTapped();
	void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint, bool faceDown = false);
	void handleDropEventByGrid(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &gridPoint, bool faceDown = false, bool tapped = false);
	CardItem *getCardFromGrid(const QPoint &gridPoint) const;
	CardItem *getCardFromCoords(const QPointF &point) const;
	QPointF mapFromGrid(QPoint gridPoint) const;
	QPoint mapToGrid(const QPointF &mapPoint) const;
	QPointF closestGridPoint(const QPointF &point);
	CardItem *takeCard(int position, int cardId, bool canResize = true);
	void resizeToContents();
	int getMinimumWidth() const { return currentMinimumWidth; }
	void setWidth(qreal _width);
	qreal getWidth() const { return width; }
	void setActive(bool _active) { active = _active; update(); }
protected:
	void addCardImpl(CardItem *card, int x, int y);
};

#endif
