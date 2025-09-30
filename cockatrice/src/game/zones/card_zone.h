/**
 * @file card_zone.h
 * @ingroup GameGraphicsZones
 * @brief TODO: Document this.
 */

#ifndef CARDZONE_H
#define CARDZONE_H

#include "../board/abstract_graphics_item.h"
#include "../board/graphics_item_type.h"
#include "logic/card_zone_logic.h"

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
    void onCardAdded(CardItem *addedCard);

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

    void setMenu(QMenu *_menu, QAction *_doubleClickAction = 0)
    {
        menu = _menu;
        doubleClickAction = _doubleClickAction;
    }

private:
    CardZoneLogic *logic;
};

#endif
