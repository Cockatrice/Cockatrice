/**
 * @file arrow_item.h
 * @ingroup GameGraphics
 */
//! \todo Document this file.

#ifndef ARROWITEM_H
#define ARROWITEM_H

#include "arrow_target.h"

#include <QGraphicsItem>
#include <QPointer>

class CardItem;
class QGraphicsSceneMouseEvent;
class QMenu;
class PlayerLogic;

class ArrowItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    QPainterPath path;

protected:
    PlayerLogic *player;
    int id;
    QPointer<ArrowTarget> startItem;
    QPointer<ArrowTarget> targetItem;
    bool targetLocked = false;
    QColor color;
    bool fullColor = true;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

public:
    ArrowItem(PlayerLogic *_player, int _id, ArrowTarget *_startItem, ArrowTarget *_targetItem, const QColor &_color);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    [[nodiscard]] QRectF boundingRect() const override
    {
        return path.boundingRect();
    }
    [[nodiscard]] QPainterPath shape() const override
    {
        return path;
    }

    void updatePath();
    void updatePath(const QPointF &endPoint);

    [[nodiscard]] int getId() const
    {
        return id;
    }
    [[nodiscard]] PlayerLogic *getPlayer() const
    {
        return player;
    }
    [[nodiscard]] ArrowTarget *getStartItem() const
    {
        return startItem;
    }
    [[nodiscard]] ArrowTarget *getTargetItem() const
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
    int deleteInPhase;
    QList<ArrowDragItem *> childArrows;
    QMetaObject::Connection positionConnection;

public:
    ArrowDragItem(PlayerLogic *_owner, ArrowTarget *_startItem, const QColor &_color, int _deleteInPhase);
    void addChildArrow(ArrowDragItem *child);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

class ArrowAttachItem : public ArrowItem
{
    Q_OBJECT
private:
    QList<ArrowAttachItem *> childArrows;
    QMetaObject::Connection positionConnection;
    void attachCards(CardItem *startCard, const CardItem *targetCard);

public:
    explicit ArrowAttachItem(ArrowTarget *_startItem);
    void addChildArrow(ArrowAttachItem *child);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // ARROWITEM_H
