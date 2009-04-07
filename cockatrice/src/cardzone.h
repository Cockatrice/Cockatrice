#ifndef CARDZONE_H
#define CARDZONE_H

#include <QString>
#include "cardlist.h"

class Player;
class ZoneViewZone;
class QMenu;

class CardZone : public QGraphicsItem {
protected:
	QPixmap *image;
	Player *player;
	QString name;
	CardList *cards;
	QList<ZoneViewZone *> views;
	QMenu *menu;
	bool hasCardAttr;
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
public:
	enum { Type = typeZone };
	int type() const { return Type; }
	virtual void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown) = 0;
	CardZone(Player *_player, const QString &_name, QGraphicsItem *parent = 0, bool isView = false);
	~CardZone();
	void clearContents();
	bool getHasCardAttr() const { return hasCardAttr; }
	QMenu *getMenu() const { return menu; }
	void setMenu(QMenu *_menu) { menu = _menu; }
	QString getName() const { return name; }
	Player *getPlayer() const { return player; }
	bool contentsKnown() const { return cards->getContentsKnown(); }
	CardList *const getCards() const { return cards; }
	virtual void addCard(CardItem *card, bool reorganize = true, int x = -1, int y = -1) = 0;
	// getCard() finds a card by id.
	CardItem *getCard(int cardId, const QString &cardName);
	// takeCard() finds a card by position and removes it from the zone and from all of its views.
	CardItem *takeCard(int position, int cardId, const QString &cardName);
	void setCardAttr(int cardId, const QString &aname, const QString &avalue);
	void hoverCardEvent(CardItem *card);
	void addView(ZoneViewZone *view);
	void removeView(ZoneViewZone *view);
	virtual void reorganizeCards() = 0;
	void moveAllToZone(const QString &targetZone, int targetX);
};

#endif
