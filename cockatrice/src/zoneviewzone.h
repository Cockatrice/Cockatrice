#ifndef ZONEVIEWERZONE_H
#define ZONEVIEWERZONE_H

#include "cardzone.h"
#include "serverzonecard.h"
#include <QGraphicsWidget>
#include <QGraphicsLayoutItem>

class ZoneViewWidget;

class ZoneViewZone : public CardZone, public QGraphicsLayoutItem {
	Q_OBJECT
private:
	int height;
	int numberCards;
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
	CardZone *origZone;
	bool sortingEnabled;
	int cmdId;
public:
	ZoneViewZone(Player *_p, CardZone *_origZone, int _numberCards = -1, QGraphicsItem *parent = 0);
	~ZoneViewZone();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void initializeCards();
	void removeCard(int position);
	void setGeometry(const QRectF &rect);
	int getNumberCards() const { return numberCards; }
public slots:
	void setSortingEnabled(int _sortingEnabled);
private slots:
	void zoneDumpReceived(int commandId, QList<ServerZoneCard *> cards);
protected:
	void addCardImpl(CardItem *card, int x, int y);
	QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
};

#endif
