#ifndef DECKVIEW_H
#define DECKVIEW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QPixmap>
#include "abstractcarditem.h"
#include "abstractcarddragitem.h"

class DeckList;
class InnerDecklistNode;
class CardInfo;
class DeckViewCardContainer;
class DeckViewCardDragItem;
class MoveCardToZone;

class DeckViewCard : public AbstractCardItem {
private:
	QString originZone;
	DeckViewCardDragItem *dragItem;
public:
	DeckViewCard(const QString &_name = QString(), const QString &_originZone = QString(), QGraphicsItem *parent = 0);
	~DeckViewCard();
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	const QString &getOriginZone() const { return originZone; }
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
};

class DeckViewCardDragItem : public AbstractCardDragItem {
private:
	DeckViewCardContainer *currentZone;
	void handleDrop(DeckViewCardContainer *target);
public:
	DeckViewCardDragItem(DeckViewCard *_item, const QPointF &_hotSpot, AbstractCardDragItem *parentDrag = 0);
	void updatePosition(const QPointF &cursorScenePos);
protected:
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

class DeckViewCardContainer : public QGraphicsItem {
private:
	QString name;
	QList<DeckViewCard *> cards;
	qreal width, height;
	qreal maxWidth;
	qreal separatorY;
	QPixmap bgPixmap;
	static const int rowSpacing = 5;
public:
	enum { Type = typeDeckViewCardContainer };
	int type() const { return Type; }
	DeckViewCardContainer(const QString &_name);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(DeckViewCard *card);
	void removeCard(DeckViewCard *card);
	const QList<DeckViewCard *> &getCards() const { return cards; }
	const QString &getName() const { return name; }
	void rearrangeItems();
	void setWidth(qreal _width);
};

class DeckViewScene : public QGraphicsScene {
	Q_OBJECT
signals:
	void newCardAdded(AbstractCardItem *card);
	void sideboardPlanChanged();
private:
	bool locked;
	DeckList *deck;
	QMap<QString, DeckViewCardContainer *> cardContainers;
	void rebuildTree();
	void applySideboardPlan(const QList<MoveCardToZone *> &plan);
	void rearrangeItems();
public:
	DeckViewScene(QObject *parent = 0);
	~DeckViewScene();
	void setLocked(bool _locked) { locked = _locked; }
	bool getLocked() const { return locked; }
	void setDeck(DeckList *_deck);
	void updateContents();
	QList<MoveCardToZone *> getSideboardPlan() const;
};

class DeckView : public QGraphicsView {
	Q_OBJECT
private:
	DeckViewScene *deckViewScene;
protected:
	void resizeEvent(QResizeEvent *event);
public slots:
	void updateSceneRect(const QRectF &rect);
signals:
	void newCardAdded(AbstractCardItem *card);
	void sideboardPlanChanged();
public:
	DeckView(QWidget *parent = 0);
	void setDeck(DeckList *_deck);
	void setLocked(bool _locked) { deckViewScene->setLocked(_locked); }
	QList<MoveCardToZone *> getSideboardPlan() const { return deckViewScene->getSideboardPlan(); }
};

#endif
