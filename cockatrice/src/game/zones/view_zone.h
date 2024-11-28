#ifndef ZONEVIEWERZONE_H
#define ZONEVIEWERZONE_H

#include "select_zone.h"

#include <QGraphicsLayoutItem>

class ZoneViewWidget;
class Response;
class ServerInfo_Card;
class QGraphicsSceneWheelEvent;

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
    bool sortByName, sortByType;
    bool pileView;

    struct GridSize
    {
        int rows;
        int cols;
    };

    GridSize positionCardsForDisplay(CardList &cards, bool groupByType = false);

public:
    ZoneViewZone(Player *_p,
                 CardZone *_origZone,
                 int _numberCards = -1,
                 bool _revealZone = false,
                 bool _writeableRevealZone = false,
                 QGraphicsItem *parent = nullptr);
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
public slots:
    void setSortByName(int _sortByName);
    void setSortByType(int _sortByType);
    void setPileView(int _pileView);
private slots:
    void zoneDumpReceived(const Response &r);
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
