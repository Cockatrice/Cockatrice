/**
 * @file card_item.h
 * @ingroup GameGraphicsCards
 * @brief TODO: Document this.
 */

#ifndef CARDITEM_H
#define CARDITEM_H

#include "../zones/logic/card_zone_logic.h"
#include "abstract_card_item.h"

#include <libcockatrice/network/server/remote/game/server_card.h>

class CardDatabase;
class CardDragItem;
class CardZone;
class ServerInfo_Card;
class Player;
class QAction;
class QColor;

const int MAX_COUNTERS_ON_CARD = 999;
const float CARD_WIDTH_HALF = CARD_WIDTH / 2;
const float CARD_HEIGHT_HALF = CARD_HEIGHT / 2;
const int ROTATION_DEGREES_PER_FRAME = 10;

class CardItem : public AbstractCardItem
{
    Q_OBJECT
private:
    CardZoneLogic *zone;
    bool attacking;
    QMap<int, int> counters;
    QString annotation;
    QString pt;
    bool destroyOnZoneChange;
    bool doesntUntap;
    QPoint gridPoint;
    CardDragItem *dragItem;
    CardItem *attachedTo;
    QList<CardItem *> attachedCards;

    void prepareDelete();
    void handleClickedToPlay(bool shiftHeld);
public slots:
    void deleteLater();

public:
    enum
    {
        Type = typeCard
    };
    [[nodiscard]] int type() const override
    {
        return Type;
    }
    explicit CardItem(Player *_owner,
                      QGraphicsItem *parent = nullptr,
                      const CardRef &cardRef = {},
                      int _cardid = -1,
                      CardZoneLogic *_zone = nullptr);

    void retranslateUi();
    [[nodiscard]] CardZoneLogic *getZone() const
    {
        return zone;
    }
    void setZone(CardZoneLogic *_zone);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    [[nodiscard]] QPoint getGridPoint() const
    {
        return gridPoint;
    }
    void setGridPoint(const QPoint &_gridPoint)
    {
        gridPoint = _gridPoint;
    }
    [[nodiscard]] QPoint getGridPos() const
    {
        return gridPoint;
    }
    [[nodiscard]] Player *getOwner() const
    {
        return owner;
    }
    void setOwner(Player *_owner)
    {
        owner = _owner;
    }
    [[nodiscard]] bool getAttacking() const
    {
        return attacking;
    }
    void setAttacking(bool _attacking);
    [[nodiscard]] const QMap<int, int> &getCounters() const
    {
        return counters;
    }
    void setCounter(int _id, int _value);
    [[nodiscard]] QString getAnnotation() const
    {
        return annotation;
    }
    void setAnnotation(const QString &_annotation);
    [[nodiscard]] bool getDoesntUntap() const
    {
        return doesntUntap;
    }
    void setDoesntUntap(bool _doesntUntap);
    [[nodiscard]] QString getPT() const
    {
        return pt;
    }
    void setPT(const QString &_pt);
    [[nodiscard]] bool getDestroyOnZoneChange() const
    {
        return destroyOnZoneChange;
    }
    void setDestroyOnZoneChange(bool _destroy)
    {
        destroyOnZoneChange = _destroy;
    }
    [[nodiscard]] CardItem *getAttachedTo() const
    {
        return attachedTo;
    }
    void setAttachedTo(CardItem *_attachedTo);
    void addAttachedCard(CardItem *card)
    {
        attachedCards.append(card);
    }
    void removeAttachedCard(CardItem *card)
    {
        attachedCards.removeOne(card);
    }
    [[nodiscard]] const QList<CardItem *> &getAttachedCards() const
    {
        return attachedCards;
    }
    void resetState(bool keepAnnotations = false);
    void processCardInfo(const ServerInfo_Card &_info);

    bool animationEvent();
    CardDragItem *createDragItem(int _id, const QPointF &_pos, const QPointF &_scenePos, bool faceDown);
    void deleteDragItem();
    void drawArrow(const QColor &arrowColor);
    void drawAttachArrow();
    void playCard(bool faceDown);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

#endif
