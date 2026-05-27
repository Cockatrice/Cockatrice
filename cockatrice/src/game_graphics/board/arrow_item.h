#ifndef ARROWITEM_H
#define ARROWITEM_H

#include "../../game/board/arrow_data.h"
#include "arrow_target.h"

#include <QGraphicsItem>
#include <QPointer>
#include <QSharedPointer>

class CardItem;
class QGraphicsSceneMouseEvent;
class PlayerLogic;

class ArrowItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
signals:
    void requestDeletion(int creatorId, int id);

private:
    QPainterPath path;

protected:
    QSharedPointer<const ArrowData> data;
    QPointer<ArrowTarget> startItem;
    QPointer<ArrowTarget> targetItem;
    bool targetLocked = false;
    bool fullColor = true;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

public:
    ArrowItem(QSharedPointer<const ArrowData> _data, ArrowTarget *_startItem, ArrowTarget *_targetItem);

    void onTargetDestroyed();
    void delArrow();
    void updatePath();
    void updatePath(const QPointF &endPoint);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    [[nodiscard]] QRectF boundingRect() const override
    {
        return path.boundingRect();
    }
    [[nodiscard]] QPainterPath shape() const override
    {
        return path;
    }
    [[nodiscard]] int getId() const
    {
        return data->id;
    }
    [[nodiscard]] int getCreatorId() const
    {
        return data->creatorId;
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
};

class ArrowDragItem : public ArrowItem
{
    Q_OBJECT
private:
    PlayerLogic *player;
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
    PlayerLogic *player;
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

#endif