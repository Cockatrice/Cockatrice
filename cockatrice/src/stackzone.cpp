#include "stackzone.h"

#include "arrowitem.h"
#include "carddragitem.h"
#include "carditem.h"
#include "pb/command_move_card.pb.h"
#include "player.h"
#include "settingscache.h"
#include "thememanager.h"

#include <QPainter>
#include <QSet>

StackZone::StackZone(Player *_p, int _zoneHeight, QGraphicsItem *parent)
    : SelectZone(_p, "stack", false, false, true, parent), zoneHeight(_zoneHeight)
{
    connect(themeManager, SIGNAL(themeChanged()), this, SLOT(updateBg()));
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
        x = cards.size();
    }
    cards.insert(x, card);

    if (!cards.getContentsKnown()) {
        card->setId(-1);
        card->setName();
    }
    card->setParentItem(this);
    card->resetState();
    card->setVisible(true);
    card->update();
}

QRectF StackZone::boundingRect() const
{
    return QRectF(0, 0, 100, zoneHeight);
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
    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(getName().toStdString());
    int new_x = qRound(cardLocationByIndex(dropPoint.y(), true));
    if (startZone == this) {
        if (cards.at(new_x)->getId() == dragItems.at(0)->getId()) {
            return;
        }
    }
    cmd.set_x(new_x);
    cmd.set_y(0);

    for (CardDragItem *item : dragItems) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(item->getId());
    }

    player->sendGameCommand(cmd);
}

qreal StackZone::cardLocationByIndex(qreal index, bool reverse)
{
    if (cards.isEmpty()) {
        return -1;
    }
    qreal totalHeight = boundingRect().height();
    qreal cardHeight = cards.at(0)->boundingRect().height();
    qreal cardMinOverlap = cardHeight / 3;
    qreal desiredHeight = cardHeight * cardCount - cardMinOverlap * (cardCount - 1);
    qreal y;
    if (desiredHeight > totalHeight) {
        if (reverse) {
            y = index / ((totalHeight - cardHeight) / (cardCount - 1));
        } else {
            y = index * (totalHeight - cardHeight) / (cardCount - 1);
        }
    } else {
        qreal start = (totalHeight - desiredHeight) / 2;
        if (reverse) {
            if (index <= start) {
                return 0;
            }
            y = (index - start) / (cardHeight - cardMinOverlap);
        } else {
            y = index * (cardHeight - cardMinOverlap) + start;
        }
    }
    return y;
}

void StackZone::reorganizeCards()
{
    cardCount = cards.size();
    if (!cards.isEmpty()) {
        QSet<ArrowItem *> arrowsToUpdate;

        const int cardCount = cards.size();
        qreal totalWidth = boundingRect().width();
        qreal cardWidth = cards.at(0)->boundingRect().width();
        qreal xspace = 5;
        qreal x1 = xspace;
        qreal x2 = totalWidth - xspace - cardWidth;

        for (int i = 0; i < cardCount; i++) {
            CardItem *card = cards.at(i);
            qreal x = (i % 2) ? x2 : x1;
            qreal y = cardLocationByIndex(i);
            card->setPos(x, y);
            card->setRealZValue(i);

            for (ArrowItem *item : card->getArrowsFrom()) {
                arrowsToUpdate.insert(item);
            }
            for (ArrowItem *item : card->getArrowsTo()) {
                arrowsToUpdate.insert(item);
            }
        }
        for (ArrowItem *item : arrowsToUpdate) {
            item->updatePath();
        }
    }
    update();
}
