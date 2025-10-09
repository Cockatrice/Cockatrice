#include "hand_zone.h"

#include "../../interface/theme_manager.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../player/player.h"

#include <QPainter>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/settings/cache_settings.h>

HandZone::HandZone(HandZoneLogic *_logic, int _zoneHeight, QGraphicsItem *parent)
    : SelectZone(_logic, parent), zoneHeight(_zoneHeight)
{
    connect(themeManager, &ThemeManager::themeChanged, this, &HandZone::updateBg);
    updateBg();
    setCacheMode(DeviceCoordinateCache);
}

void HandZone::updateBg()
{
    update();
}

void HandZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                               CardZoneLogic *startZone,
                               const QPoint &dropPoint)
{
    QPoint point = dropPoint + scenePos().toPoint();
    int x = -1;
    if (SettingsCache::instance().getHorizontalHand()) {
        for (x = 0; x < getLogic()->getCards().size(); x++)
            if (point.x() < static_cast<CardItem *>(getLogic()->getCards().at(x))->scenePos().x())
                break;
    } else {
        for (x = 0; x < getLogic()->getCards().size(); x++)
            if (point.y() < static_cast<CardItem *>(getLogic()->getCards().at(x))->scenePos().y())
                break;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(getLogic()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_target_zone(getLogic()->getName().toStdString());
    cmd.set_x(x);
    cmd.set_y(-1);

    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());

    getLogic()->getPlayer()->getPlayerActions()->sendGameCommand(cmd);
}

QRectF HandZone::boundingRect() const
{
    if (SettingsCache::instance().getHorizontalHand())
        return QRectF(0, 0, width, CARD_HEIGHT + 10);
    else
        return QRectF(0, 0, 100, zoneHeight);
}

void HandZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QBrush brush = themeManager->getExtraBgBrush(ThemeManager::Hand, getLogic()->getPlayer()->getZoneId());
    painter->fillRect(boundingRect(), brush);
}

void HandZone::reorganizeCards()
{
    if (!getLogic()->getCards().isEmpty()) {
        const int cardCount = getLogic()->getCards().size();
        if (SettingsCache::instance().getHorizontalHand()) {
            bool leftJustified = SettingsCache::instance().getLeftJustified();
            qreal cardWidth = getLogic()->getCards().at(0)->boundingRect().width();
            const int xPadding = leftJustified ? cardWidth * 1.4 : 5;
            qreal totalWidth =
                leftJustified ? boundingRect().width() - (1 * xPadding) - 5 : boundingRect().width() - 2 * xPadding;

            for (int i = 0; i < cardCount; i++) {
                CardItem *c = getLogic()->getCards().at(i);
                // If the total width of the cards is smaller than the available width,
                // the cards do not need to overlap and are displayed in the center of the area.
                if (cardWidth * cardCount > totalWidth)
                    c->setPos(xPadding + ((qreal)i) * (totalWidth - cardWidth) / (cardCount - 1), 5);
                else {
                    qreal xPosition =
                        leftJustified ? xPadding + ((qreal)i) * cardWidth
                                      : xPadding + ((qreal)i) * cardWidth + (totalWidth - cardCount * cardWidth) / 2;
                    c->setPos(xPosition, 5);
                }
                c->setRealZValue(i);
            }
        } else {
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
    }
    update();
}

void HandZone::sortHand()
{
    if (getLogic()->getCards().isEmpty()) {
        return;
    }
    getLogic()->sortCards({CardList::SortByMainType, CardList::SortByManaValue, CardList::SortByColorGrouping});
    reorganizeCards();
}

void HandZone::setWidth(qreal _width)
{
    if (SettingsCache::instance().getHorizontalHand()) {
        prepareGeometryChange();
        width = _width;
        reorganizeCards();
    }
}

void HandZone::updateOrientation()
{
    prepareGeometryChange();
    reorganizeCards();
}
