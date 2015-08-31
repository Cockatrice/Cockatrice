#ifndef DECKVIEW_H
#define DECKVIEW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QMultiMap>
#include <QPixmap>
#include "abstractcarddragitem.h"

#include "pb/move_card_to_zone.pb.h"

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
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
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
    static const int separatorY = 20;
    static const int paddingY = 10;
    static bool sortCardsByName(DeckViewCard * c1, DeckViewCard * c2);
    
    QString name;
    QList<DeckViewCard *> cards;
    QMultiMap<QString, DeckViewCard *> cardsByType;
    QList<QPair<int, int> > currentRowsAndCols;
    qreal width, height;
    int getCardTypeTextWidth() const;
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
    void setWidth(qreal _width);

    QList<QPair<int, int> > getRowsAndCols() const;
    QSizeF calculateBoundingRect(const QList<QPair<int, int> > &rowsAndCols) const;
    void rearrangeItems(const QList<QPair<int, int> > &rowsAndCols);
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
    qreal optimalAspectRatio;
    void clearContents();
    void rebuildTree();
    void applySideboardPlan(const QList<MoveCard_ToZone> &plan);
public:
    DeckViewScene(QObject *parent = 0);
    ~DeckViewScene();
    void setLocked(bool _locked) { locked = _locked; }
    bool getLocked() const { return locked; }
    void setDeck(const DeckList &_deck);
    void setOptimalAspectRatio(qreal _optimalAspectRatio) { optimalAspectRatio = _optimalAspectRatio; }
    void rearrangeItems();
    void updateContents();
    QList<MoveCard_ToZone> getSideboardPlan() const;
    void resetSideboardPlan();
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
    void setDeck(const DeckList &_deck);
    void setLocked(bool _locked) { deckViewScene->setLocked(_locked); }
    QList<MoveCard_ToZone> getSideboardPlan() const { return deckViewScene->getSideboardPlan(); }
    void resetSideboardPlan();
};

#endif
