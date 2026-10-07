#include "stack_zone.h"

#include "../../interface/theme_manager.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../card_dimensions.h"

#include <QPainter>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/zones/stack_zone_logic.h>

StackZone::StackZone(StackZoneLogic *_logic, int _zoneHeight, QGraphicsItem *parent)
    : SelectZone(_logic, parent), zoneHeight(_zoneHeight)
{
    connect(themeManager, &ThemeManager::themeChanged, this, &StackZone::updateBg);
    updateBg();
    setCacheMode(DeviceCoordinateCache);
}

void StackZone::updateBg()
{
    update();
}

QRectF StackZone::boundingRect() const
{
    return {0, 0, CardDimensions::WIDTH_F * 1.5, zoneHeight};
}

void StackZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if (playmatActive) {
        // Subtle overlay to distinguish stack zone from table zone (slightly darker)
        painter->fillRect(boundingRect(), QColor(0, 0, 0, 80));
    } else {
        QBrush brush = themeManager->getExtraBgBrush(ThemeManager::Stack, getLogic()->getPlayer()->getZoneId());
        painter->fillRect(boundingRect(), brush);
    }
}

void StackZone::onPlaymatChanged(bool active)
{
    playmatActive = active;
    // See TableZone::onPlaymatChanged for the rationale. Translucent overlay
    // over a dynamic playmat should not be held in the device cache.
    setCacheMode(active ? QGraphicsItem::NoCache : QGraphicsItem::DeviceCoordinateCache);
    update();
}

void StackZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                CardZoneLogic *startZone,
                                const QPoint &dropPoint)
{
    if (startZone == nullptr || startZone->getPlayer() == nullptr || dragItems.isEmpty()) {
        return;
    }

    bool sameZone = startZone == getLogic();
    int index = calcDropIndexFromY(dropPoint.y(), !sameZone, MIN_CARD_VISIBLE);
    if (sameZone) {
        // Same-zone no-op: don't move a card onto itself
        const auto &cards = cardItems();
        if (!cards.isEmpty() && cards.at(index)->getId() == dragItems.at(0)->getId()) {
            return;
        }
    }

    QList<CardMoveRequest> cards;
    cards.reserve(dragItems.size());
    for (const CardDragItem *item : dragItems) {
        if (item) {
            cards.append(CardMoveRequest{item->getId(), item->isForceFaceDown()});
        }
    }

    getLogic()->getPlayer()->getPlayerActions()->moveCards(startZone, getLogic(), index, 0, cards);
}

void StackZone::setHeight(qreal newHeight)
{
    if (qFuzzyCompare(1.0 + zoneHeight, 1.0 + newHeight)) {
        return;
    }
    prepareGeometryChange();
    zoneHeight = newHeight;
    reorganizeCards();
    update();
}

void StackZone::reorganizeCards()
{
    if (!cardItems().isEmpty()) {
        const auto params = buildStackParams(MIN_CARD_VISIBLE);
        layoutCardsVertically(params);
    }
    update();
}
