#ifndef ZONEVIEWERZONE_H
#define ZONEVIEWERZONE_H

#include "select_zone.h"

#include <QGraphicsLayoutItem>
#include <pb/commands.pb.h>

class ZoneViewWidget;
class Response;
class ServerInfo_Card;
class QGraphicsSceneWheelEvent;

/**
 * A CardZone that is a view into another CardZone.
 * If this zone is writable, then modifications to this zone will be reflected in the underlying zone.
 *
 * Most interactions with StackZones are handled through a zone view.
 * For example, viewing the deck/graveyard/exile is handled through a view.
 *
 * Handles both limited reveals (eg. look at top X cards) and full zone reveals (eg. searching the deck).
 */
class ZoneViewZone : public SelectZone, public QGraphicsLayoutItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsLayoutItem)
private:
    static constexpr int HORIZONTAL_PADDING = 12;
    static constexpr int VERTICAL_PADDING = 5;

    QRectF bRect, optimumRect;
    int minRows, numberCards;
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    CardZone *origZone;
    bool revealZone, writeableRevealZone;
    CardList::SortOption groupBy, sortBy;
    bool pileView;
    bool isReversed;

    enum CardAction
    {
        INITIALIZE,
        ADD_CARD,
        REMOVE_CARD
    };

    void updateCardIds(CardAction action);

    struct GridSize
    {
        int rows;
        int cols;
    };

    GridSize positionCardsForDisplay(CardList &cards, CardList::SortOption pileOption = CardList::NoSort);

public:
    ZoneViewZone(Player *_p,
                 CardZone *_origZone,
                 int _numberCards = -1,
                 bool _revealZone = false,
                 bool _writeableRevealZone = false,
                 QGraphicsItem *parent = nullptr,
                 bool _isReversed = false);
    ~ZoneViewZone();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void reorganizeCards();
    void initializeCards(const QList<const ServerInfo_Card *> &cardList = QList<const ServerInfo_Card *>());
    void removeCard(int position);
    int getNumberCards() const
    {
        return numberCards;
    }
    void setGeometry(const QRectF &rect);
    QRectF getOptimumRect() const
    {
        return optimumRect;
    }
    bool getRevealZone() const
    {
        return revealZone;
    }
    bool getWriteableRevealZone() const
    {
        return writeableRevealZone;
    }
    void setWriteableRevealZone(bool _writeableRevealZone);
    bool getIsReversed() const
    {
        return isReversed;
    }
public slots:
    void setGroupBy(CardList::SortOption _groupBy);
    void setSortBy(CardList::SortOption _sortBy);
    void setPileView(int _pileView);
private slots:
    void zoneDumpReceived(const Response &r, const CommandContainer &, const QVariant &);
signals:
    void beingDeleted();
    void optimumRectChanged();
    void wheelEventReceived(QGraphicsSceneWheelEvent *event);

protected:
    void addCardImpl(CardItem *card, int x, int y);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    void wheelEvent(QGraphicsSceneWheelEvent *event);
};

#endif
