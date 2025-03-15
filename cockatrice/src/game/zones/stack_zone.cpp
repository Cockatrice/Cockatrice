#include "stack_zone.h"

#include "../../client/ui/theme_manager.h"
#include "../../settings/cache_settings.h"
#include "../board/arrow_item.h"
#include "../cards/card_drag_item.h"
#include "../cards/card_item.h"
#include "../player/player.h"
#include "pb/command_move_card.pb.h"

#include <QPainter>
#include <QSet>

StackZone::StackZone(Player *_p, int _zoneHeight, QGraphicsItem *parent)
    : SelectZone(_p, "stack", false, false, true, parent), zoneHeight(_zoneHeight)
{
    connect(themeManager, &ThemeManager::themeChanged, this, &StackZone::updateBg);
    updateBg();
    setCacheMode(DeviceCoordinateCache);
}

void StackZone::updateBg()
{
    update();
}

void StackZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
    // if x is negative set it to add at end
    if (x < 0 || x >= cards.size()) {
        x = static_cast<int>(cards.size());
    }
    cards.insert(x, card);

    if (!cards.getContentsKnown()) {
        card->setId(-1);
        card->setName();
    }
    card->setParentItem(this);
    card->resetState(true);
    card->setVisible(true);
    card->update();
}

QRectF StackZone::boundingRect() const
{
    return {0, 0, 100, zoneHeight};
}

void StackZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QBrush brush = themeManager->getStackBgBrush();

    if (player->getZoneId() > 0) {
        // If the extra image is not found, load the default one
        brush = themeManager->getExtraStackBgBrush(QString::number(player->getZoneId()), brush);
    }
    painter->fillRect(boundingRect(), brush);
}

void StackZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint)
{
    if (startZone == nullptr || startZone->getPlayer() == nullptr) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(getName().toStdString());

    int index = 0;
    if (cards.isEmpty()) {
        index = 0;
    } else {
        const auto cardCount = static_cast<int>(cards.size());
        const auto &card = cards.at(0);

        if (card == nullptr) {
            qCWarning(StackZoneLog) << "Attempted to move card from" << startZone->getName() << ", but was null";
            return;
        }

        index = qRound(divideCardSpaceInZone(dropPoint.y(), cardCount, boundingRect().height(),
                                             card->boundingRect().height(), true));
    }

    if (startZone == this) {
        const auto &dragItem = dragItems.at(0);
        const auto &card = cards.at(index);
        if (card != nullptr && dragItem != nullptr && card->getId() == dragItem->getId()) {
            return;
        }
    }

    cmd.set_x(index);
    cmd.set_y(0);

    for (CardDragItem *item : dragItems) {
        if (item) {
            cmd.mutable_cards_to_move()->add_card()->set_card_id(item->getId());
        }
    }

    player->sendGameCommand(cmd);
}

void StackZone::reorganizeCards()
{
    if (!cards.isEmpty()) {
        const auto cardCount = static_cast<int>(cards.size());
        qreal totalWidth = boundingRect().width();
        qreal cardWidth = cards.at(0)->boundingRect().width();
        qreal xspace = 5;
        qreal x1 = xspace;
        qreal x2 = totalWidth - xspace - cardWidth;

        for (int i = 0; i < cardCount; i++) {
            CardItem *card = cards.at(i);
            qreal x = (i % 2) ? x2 : x1;
            qreal y =
                divideCardSpaceInZone(i, cardCount, boundingRect().height(), cards.at(0)->boundingRect().height());
            card->setPos(x, y);
            card->setRealZValue(i);
        }
    }
    update();
}
