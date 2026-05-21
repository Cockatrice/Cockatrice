/**
 * @file card_zone.h
 * @ingroup GameGraphicsZones
 * @brief Base graphics item for zones that contain cards.
 */

#ifndef CARDZONE_H
#define CARDZONE_H

#include "../../game/zones/card_zone_logic.h"
#include "../board/abstract_graphics_item.h"
#include "../board/card_item.h"
#include "../board/graphics_item_type.h"

#include <QLoggingCategory>
#include <QString>

inline Q_LOGGING_CATEGORY(CardZoneLog, "card_zone");

/**
 * A zone in the game that can contain cards.
 * This class contains methods to get and modify the cards that are contained inside this zone.
 *
 * The cards are stored as a list of `CardItem*`.
 */
class CardZone : public AbstractGraphicsItem
{
    Q_OBJECT
protected:
    CardZoneLogic *logic;
    QList<CardItem *> cards;

    QMenu *menu;
    QAction *doubleClickAction;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void reorganizeCards() = 0;
    virtual QPointF closestGridPoint(const QPointF &point);

    QMenu *getMenu() const
    {
        return menu;
    }
public slots:
    bool showContextMenu(const QPoint &screenPos);
    /**
     * @brief Called when a card is added to this zone. Default: reparents card to this item.
     *
     * Virtual so subclasses (e.g. SelectZone) can override parenting behavior — the Qt signal
     * connection in CardZone's constructor dispatches through the vtable.
     */
    virtual void onCardAdded(CardState *addedCard, int x, int y);

signals:
    void cardItemAdded(CardItem *added);

public:
    enum
    {
        Type = typeZone
    };
    int type() const override
    {
        return Type;
    }
    virtual void
    handleDropEvent(const QList<CardDragItem *> &dragItem, CardZoneLogic *startZone, const QPoint &dropPoint) = 0;
    CardZone(CardZoneLogic *logic, QGraphicsItem *parent = nullptr);
    void retranslateUi();

    CardZoneLogic *getLogic() const
    {
        return logic;
    }

    CardItem *getCardItemForId(int id) const
    {
        for (CardItem *card : cards) {
            if (card->getState()->getId() == id) {
                return card;
            }
        }
        return nullptr;
    }

    void setMenu(QMenu *_menu, QAction *_doubleClickAction = 0)
    {
        menu = _menu;
        doubleClickAction = _doubleClickAction;
    }
};

#endif
