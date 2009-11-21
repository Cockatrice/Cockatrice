#ifndef DECKVIEW_H
#define DECKVIEW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QPixmap>
#include "carditem.h"

class DeckList;
class InnerDecklistNode;
class CardInfo;

class DeckViewCard : public AbstractCardItem {
public:
	DeckViewCard(const QString &_name = QString(), QGraphicsItem *parent = 0);
};

class DeckViewCardContainer : public QGraphicsItem {
private:
	QString name;
	QMap<QString, DeckViewCard *> cards;
	qreal width, height;
	qreal separatorY;
	QPixmap bgPixmap;
	static const int rowSpacing = 5;
public:
	DeckViewCardContainer(const QString &_name);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(DeckViewCard *card);
	void rearrangeItems();
	void setWidth(qreal _width);
};

class DeckViewScene : public QGraphicsScene {
	Q_OBJECT
private:
	DeckList *deck;
	QMap<QString, DeckViewCardContainer *> cardContainers;
	void rebuildTree();
	void rearrangeItems();
public:
	DeckViewScene(QObject *parent = 0);
	~DeckViewScene();
	void setDeck(DeckList *_deck);
};

class DeckView : public QGraphicsView {
	Q_OBJECT
private:
	DeckViewScene *deckViewScene;
protected:
	void resizeEvent(QResizeEvent *event);
public slots:
	void updateSceneRect(const QRectF &rect);
public:
	DeckView(QWidget *parent = 0);
	void setDeck(DeckList *_deck);
};

#endif
