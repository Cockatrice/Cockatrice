/**
 * @file deck_view.h
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef DECKVIEW_H
#define DECKVIEW_H

#include "../board/abstract_card_drag_item.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QMultiMap>
#include <QPixmap>
#include <libcockatrice/protocol/pb/move_card_to_zone.pb.h>

class DeckList;
class InnerDecklistNode;
class CardInfo;
class DeckViewCardContainer;
class DeckViewCardDragItem;
class MoveCardToZone;

class DeckViewCard : public AbstractCardItem
{
private:
    QString originZone;
    DeckViewCardDragItem *dragItem;

public:
    explicit DeckViewCard(QGraphicsItem *parent = nullptr,
                          const CardRef &cardRef = {},
                          const QString &_originZone = QString());
    ~DeckViewCard() override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    [[nodiscard]] const QString &getOriginZone() const
    {
        return originZone;
    }

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
};

class DeckViewCardDragItem : public AbstractCardDragItem
{
private:
    DeckViewCardContainer *currentZone;
    void handleDrop(DeckViewCardContainer *target);

public:
    DeckViewCardDragItem(DeckViewCard *_item, const QPointF &_hotSpot, AbstractCardDragItem *parentDrag = 0);
    void updatePosition(const QPointF &cursorScenePos) override;

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

class DeckViewCardContainer : public QGraphicsItem
{
private:
    static const int separatorY = 20;
    static const int paddingY = 10;
    static bool sortCardsByName(DeckViewCard *c1, DeckViewCard *c2);

    QString name;
    QList<DeckViewCard *> cards;
    QMultiMap<QString, DeckViewCard *> cardsByType;
    QList<QPair<int, int>> currentRowsAndCols;
    qreal width, height;
    [[nodiscard]] int getCardTypeTextWidth() const;

public:
    enum
    {
        Type = typeDeckViewCardContainer
    };
    [[nodiscard]] int type() const override
    {
        return Type;
    }
    explicit DeckViewCardContainer(const QString &_name);
    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void addCard(DeckViewCard *card);
    void removeCard(DeckViewCard *card);
    [[nodiscard]] const QList<DeckViewCard *> &getCards() const
    {
        return cards;
    }
    [[nodiscard]] const QString &getName() const
    {
        return name;
    }
    void setWidth(qreal _width);

    [[nodiscard]] QList<QPair<int, int>> getRowsAndCols() const;
    [[nodiscard]] QSizeF calculateBoundingRect(const QList<QPair<int, int>> &rowsAndCols) const;
    void rearrangeItems(const QList<QPair<int, int>> &rowsAndCols);
};

class DeckViewScene : public QGraphicsScene
{
    Q_OBJECT
signals:
    void newCardAdded(AbstractCardItem *card);
    void sideboardPlanChanged();

private:
    bool locked;
    DeckList *deck;
    QMap<QString, DeckViewCardContainer *> cardContainers;
    qreal optimalAspectRatio;
    void rebuildTree();

public:
    explicit DeckViewScene(QObject *parent = nullptr);
    ~DeckViewScene() override;
    void setLocked(bool _locked)
    {
        locked = _locked;
    }
    [[nodiscard]] bool getLocked() const
    {
        return locked;
    }
    void clearContents();
    void setDeck(const DeckList &_deck);
    void setOptimalAspectRatio(qreal _optimalAspectRatio)
    {
        optimalAspectRatio = _optimalAspectRatio;
    }
    void rearrangeItems();
    void updateContents();
    [[nodiscard]] QList<MoveCard_ToZone> getSideboardPlan() const;
    void resetSideboardPlan();
    void applySideboardPlan(const QList<MoveCard_ToZone> &plan);
};

class DeckView : public QGraphicsView
{
    Q_OBJECT
private:
    DeckViewScene *deckViewScene;

protected:
    void resizeEvent(QResizeEvent *event) override;
public slots:
    void updateSceneRect(const QRectF &rect);
signals:
    void newCardAdded(AbstractCardItem *card);
    void sideboardPlanChanged();

public:
    explicit DeckView(QWidget *parent = nullptr);
    void setDeck(const DeckList &_deck);
    void clearDeck();
    void setLocked(bool _locked)
    {
        deckViewScene->setLocked(_locked);
    }
    [[nodiscard]] QList<MoveCard_ToZone> getSideboardPlan() const
    {
        return deckViewScene->getSideboardPlan();
    }
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resetSideboardPlan();
};

#endif
