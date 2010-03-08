#ifndef ZONEVIEWERZONE_H
#define ZONEVIEWERZONE_H

#include "cardzone.h"
#include <QGraphicsLayoutItem>

class ZoneViewWidget;
class ProtocolResponse;

class ZoneViewZone : public CardZone, public QGraphicsLayoutItem {
	Q_OBJECT
private:
	QRectF bRect, optimumRect;
	int minRows, numberCards;
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
	CardZone *origZone;
	bool sortByName, sortByType;
public:
	enum { Type = typeZoneView };
	int type() const { return Type; }
	ZoneViewZone(Player *_p, CardZone *_origZone, int _numberCards = -1, QGraphicsItem *parent = 0);
	~ZoneViewZone();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void initializeCards();
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
