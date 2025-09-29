/**
 * @file arrow_item.h
 * @ingroup GameGraphics
 * @brief TODO: Document this.
 */

#ifndef ARROWITEM_H
#define ARROWITEM_H

#include <QGraphicsItem>

class CardItem;
class QGraphicsSceneMouseEvent;
class QMenu;
class Player;
class ArrowTarget;

class ArrowItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    QPainterPath path;
    QMenu *menu;

protected:
    Player *player;
    int id;
    ArrowTarget *startItem, *targetItem;
    bool targetLocked;
    QColor color;
    bool fullColor;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

public:
    ArrowItem(Player *_player, int _id, ArrowTarget *_startItem, ArrowTarget *_targetItem, const QColor &color);
    ~ArrowItem() override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override
    {
        return path.boundingRect();
    }
    QPainterPath shape() const override
    {
        return path;
    }
    void updatePath();
    void updatePath(const QPointF &endPoint);

    int getId() const
    {
        return id;
    }
    Player *getPlayer() const
    {
        return player;
    }
    void setStartItem(ArrowTarget *_item)
    {
        startItem = _item;
    }
    void setTargetItem(ArrowTarget *_item)
    {
        targetItem = _item;
    }
    ArrowTarget *getStartItem() const
    {
        return startItem;
    }
    ArrowTarget *getTargetItem() const
    {
        return targetItem;
    }
    void setTargetLocked(bool _targetLocked)
    {
        targetLocked = _targetLocked;
    }
    void delArrow();
};

class ArrowDragItem : public ArrowItem
{
    Q_OBJECT
private:
    QList<ArrowDragItem *> childArrows;

public:
    ArrowDragItem(Player *_owner, ArrowTarget *_startItem, const QColor &_color);
    void addChildArrow(ArrowDragItem *childArrow);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

class ArrowAttachItem : public ArrowItem
{
    Q_OBJECT
private:
    QList<ArrowAttachItem *> childArrows;

    void attachCards(CardItem *startCard, const CardItem *targetCard);

public:
    explicit ArrowAttachItem(ArrowTarget *_startItem);
    void addChildArrow(ArrowAttachItem *childArrow);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // ARROWITEM_H
