/**
 * @file card_item.h
 * @ingroup GameGraphicsCards
 */
//! \todo Document this file.

#ifndef CARDITEM_H
#define CARDITEM_H

#include "../zones/card_zone_logic.h"
#include "abstract_card_item.h"
#include "card_state.h"

#include <libcockatrice/network/server/remote/game/server_card.h>
#include <libcockatrice/utility/trice_limits.h>

class CardDatabase;
class CardDragItem;
class CardZone;
class ServerInfo_Card;
class PlayerLogic;
class QAction;
class QColor;

const int ROTATION_DEGREES_PER_FRAME = 10;

class CardItem : public AbstractCardItem
{
    Q_OBJECT
private:
    CardState *state;

    QPoint gridPoint;
    CardDragItem *dragItem;
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
    explicit CardItem(PlayerLogic *_owner,
                      QGraphicsItem *parent = nullptr,
                      const CardRef &cardRef = {},
                      int _cardid = -1,
                      CardZoneLogic *_zone = nullptr);

    void retranslateUi();
    [[nodiscard]] CardState *getState() const
    {
        return state;
    }
    [[nodiscard]] CardZoneLogic *getZone() const
    {
        return state->getZone();
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
    [[nodiscard]] PlayerLogic *getOwner() const
    {
        return owner;
    }
    void setOwner(PlayerLogic *_owner)
    {
        owner = _owner;
    }
    [[nodiscard]] bool getAttacking() const
    {
        return state->getAttacking();
    }
    void setAttacking(bool _attacking);
    [[nodiscard]] const QMap<int, int> &getCounters() const
    {
        return state->getCounters();
    }
    void setCounter(int _id, int _value);
    [[nodiscard]] QString getAnnotation() const
    {
        return state->getAnnotation();
    }
    void setAnnotation(const QString &_annotation);
    [[nodiscard]] bool getDoesntUntap() const
    {
        return state->getDoesntUntap();
    }
    void setDoesntUntap(bool _doesntUntap);
    [[nodiscard]] QString getPT() const
    {
        return state->getPT();
    }
    void setPT(const QString &_pt);
    [[nodiscard]] bool getDestroyOnZoneChange() const
    {
        return state->getDestroyOnZoneChange();
    }
    void setDestroyOnZoneChange(bool _destroy)
    {
        state->setDestroyOnZoneChange(_destroy);
    }
    [[nodiscard]] CardItem *getAttachedTo() const
    {
        return state->getAttachedTo();
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
    CardDragItem *createDragItem(int _id, const QPointF &_pos, const QPointF &_scenePos, bool forceFaceDown);
    void deleteDragItem();
    void drawArrow(const QColor &arrowColor);
    void drawAttachArrow();
    void playCard(bool faceDown);

    /**
     * @brief Parses a string representing a p/t in order to extract the values from it.
     *
     * If the string contains '/', the string will be split at the '/' and each side will be parsed separately,
     * which means the result list will have two elements.
     *
     * If '/' is not found, then the entire string is parsed together, which means the result list will
     * have a single element.
     *
     * If either side of the split is empty, there will also only be a single element in the result list.
     *
     * This function will attempt to parse each substring as an int first, handling plus and minus prefixes.
     * If successful, it will put the parsed value into the QVariant as an int.
     * If failed, it will just put the substring into the QVariant as a QString.
     *
     * @param pt The p/t string
     * @return A QVariantList that can contain one or two elements, where each QVariant can be either int or QString
     */
    static QVariantList parsePT(const QString &pt);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

#endif
