#ifndef CARDZONE_H
#define CARDZONE_H

#include <QString>
#include "cardlist.h"
#include "carditem.h"
#include "abstractgraphicsitem.h"
#include "translation.h"

class Player;
class ZoneViewZone;
class QMenu;
class QAction;
class QPainter;
class CardDragItem;

class CardZone : public AbstractGraphicsItem {
	Q_OBJECT
protected:
	Player *player;
	QString name;
	CardList cards;
	ZoneViewZone *view;
	QMenu *menu;
	QAction *doubleClickAction;
	bool hasCardAttr;
	bool isShufflable;
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void addCardImpl(CardItem *card, int x, int y) = 0;
signals:
	void cardCountChanged();
public slots:
	void moveAllToZone();
public:
	enum { Type = typeZone };
	int type() const { return Type; }
	virtual void handleDropEvent(const QList<CardDragItem *> &dragItem, CardZone *startZone, const QPoint &dropPoint, bool faceDown) = 0;
	CardZone(Player *_player, const QString &_name, bool _hasCardAttr, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent = 0, bool isView = false);
	~CardZone();
	void retranslateUi();
	void clearContents();
	bool getHasCardAttr() const { return hasCardAttr; }
	bool getIsShufflable() const { return isShufflable; }
	QMenu *getMenu() const { return menu; }
	void setMenu(QMenu *_menu, QAction *_doubleClickAction = 0) { menu = _menu; doubleClickAction = _doubleClickAction; }
	QString getName() const { return name; }
	QString getTranslatedName(bool hisOwn, GrammaticalCase gc) const;
	Player *getPlayer() const { return player; }
	bool contentsKnown() const { return cards.getContentsKnown(); }
	const CardList &getCards() const { return cards; }
	void addCard(CardItem *card, bool reorganize, int x, int y = -1);
	// getCard() finds a card by id.
	CardItem *getCard(int cardId, const QString &cardName);
	// takeCard() finds a card by position and removes it from the zone and from all of its views.
	virtual CardItem *takeCard(int position, int cardId, bool canResize = true);
	void removeCard(CardItem *card);
	ZoneViewZone *getView() const { return view; }
	void setView(ZoneViewZone *_view) { view = _view; }
	virtual void reorganizeCards() = 0;
	virtual QPointF closestGridPoint(const QPointF &point);
};

#endif
