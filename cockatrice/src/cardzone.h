#ifndef CARDZONE_H
#define CARDZONE_H

#include <QString>
#include "cardlist.h"

class Player;
class ZoneViewZone;
class QMenu;
class QPainter;

class CardZone : public QGraphicsItem {
protected:
	Player *player;
	QString name;
	CardList *cards;
	ZoneViewZone *view;
	QMenu *menu;
	bool hasCardAttr;
	bool isShufflable;
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void addCardImpl(CardItem *card, int x, int y) = 0;
	void paintCardNumberEllipse(QPainter *painter);
public:
	enum { Type = typeZone };
	int type() const { return Type; }
	virtual void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown) = 0;
	CardZone(Player *_player, const QString &_name, bool _hasCardAttr, bool _isShufflable, QGraphicsItem *parent = 0, bool isView = false);
	~CardZone();
	void clearContents();
	bool getHasCardAttr() const { return hasCardAttr; }
	bool getIsShufflable() const { return isShufflable; }
	QMenu *getMenu() const { return menu; }
	void setMenu(QMenu *_menu) { menu = _menu; }
	QString getName() const { return name; }
	Player *getPlayer() const { return player; }
	bool contentsKnown() const { return cards->getContentsKnown(); }
	CardList *getCards() const { return cards; }
	void addCard(CardItem *card, bool reorganize, int x, int y = -1);
	// getCard() finds a card by id.
	CardItem *getCard(int cardId, const QString &cardName);
	// takeCard() finds a card by position and removes it from the zone and from all of its views.
	CardItem *takeCard(int position, int cardId, const QString &cardName);
	void setCardAttr(int cardId, const QString &aname, const QString &avalue);
	void hoverCardEvent(CardItem *card);
	ZoneViewZone *getView() const { return view; }
	void setView(ZoneViewZone *_view);
	virtual void reorganizeCards() = 0;
	void moveAllToZone(const QString &targetZone, int targetX);
};

#endif
