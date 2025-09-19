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
    [[nodiscard]] Player *getPlayer() const
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

public:
    ArrowDragItem(Player *_owner, ArrowTarget *_startItem, const QColor &_color, int _deleteInPhase);
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
