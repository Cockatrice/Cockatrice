#include "stack_zone.h"

#include "../../client/settings/cache_settings.h"
#include "../../interface/theme_manager.h"
#include "../board/arrow_item.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../player/player.h"
#include "../player/player_actions.h"
#include "logic/stack_zone_logic.h"

#include <QPainter>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>

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
    return {0, 0, 100, zoneHeight};
}

void StackZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QBrush brush = themeManager->getExtraBgBrush(ThemeManager::Stack, getLogic()->getPlayer()->getZoneId());
    painter->fillRect(boundingRect(), brush);
}

void StackZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                CardZoneLogic *startZone,
                                const QPoint &dropPoint)
{
    if (startZone == nullptr || startZone->getPlayer() == nullptr) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(getLogic()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_target_zone(getLogic()->getName().toStdString());

    int index = 0;

    if (!getLogic()->getCards().isEmpty()) {
        const auto cardCount = static_cast<int>(getLogic()->getCards().size());
        const auto &card = getLogic()->getCards().at(0);

        index = qRound(divideCardSpaceInZone(dropPoint.y(), cardCount, boundingRect().height(),
                                             card->boundingRect().height(), true));

        // divideCardSpaceInZone is not guaranteed to return a valid index
        // currently, so clamp it to avoid crashes.
        index = qBound(0, index, cardCount - 1);

        if (startZone == getLogic()) {
            const auto &dragItem = dragItems.at(0);
            const auto &card = getLogic()->getCards().at(index);

            if (card->getId() == dragItem->getId()) {
                return;
            }
        }
    }

    cmd.set_x(index);
    cmd.set_y(0);

    for (CardDragItem *item : dragItems) {
        if (item) {
            cmd.mutable_cards_to_move()->add_card()->set_card_id(item->getId());
        }
    }

    getLogic()->getPlayer()->getPlayerActions()->sendGameCommand(cmd);
}

void StackZone::reorganizeCards()
{
    if (!getLogic()->getCards().isEmpty()) {
        const auto cardCount = static_cast<int>(getLogic()->getCards().size());
        qreal totalWidth = boundingRect().width();
        qreal cardWidth = getLogic()->getCards().at(0)->boundingRect().width();
        qreal xspace = 5;
        qreal x1 = xspace;
        qreal x2 = totalWidth - xspace - cardWidth;

        for (int i = 0; i < cardCount; i++) {
            CardItem *card = getLogic()->getCards().at(i);
            qreal x = (i % 2) ? x2 : x1;
            qreal y = divideCardSpaceInZone(i, cardCount, boundingRect().height(),
                                            getLogic()->getCards().at(0)->boundingRect().height());
            card->setPos(x, y);
            card->setRealZValue(i);
        }
    }
    update();
}
