#include <QPainter>
#include "handzone.h"
#include "settingscache.h"
#include "thememanager.h"
#include "player.h"
#include "carddragitem.h"
#include "carditem.h"

#include "pb/command_move_card.pb.h"

HandZone::HandZone(Player *_p, bool _contentsKnown, int _zoneHeight, QGraphicsItem *parent)
    : SelectZone(_p, "hand", false, false, _contentsKnown, parent), zoneHeight(_zoneHeight)
{
    connect(themeManager, SIGNAL(themeChanged()), this, SLOT(updateBg()));
    updateBg();
    setCacheMode(DeviceCoordinateCache);
}

void HandZone::updateBg()
{
    update();
}

void HandZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
    if (x == -1)
        x = cards.size();
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

void HandZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint & dropPoint)
{
    QPoint point = dropPoint + scenePos().toPoint();
    int x = -1;
    if (settingsCache->getHorizontalHand()) {
        for (x = 0; x < cards.size(); x++)
            if (point.x() < ((CardItem *) cards.at(x))->scenePos().x())
                break;
    } else {
        for (x = 0; x < cards.size(); x++)
            if (point.y() < ((CardItem *) cards.at(x))->scenePos().y())
                break;
    }
    
    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(getName().toStdString());
    cmd.set_x(x);
    cmd.set_y(-1);
    
    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());
    
    player->sendGameCommand(cmd);
}

QRectF HandZone::boundingRect() const
{
    if (settingsCache->getHorizontalHand())
        return QRectF(0, 0, width, CARD_HEIGHT + 10);
    else
        return QRectF(0, 0, 100, zoneHeight);
}

void HandZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->fillRect(boundingRect(), themeManager->getHandBgBrush());
}

void HandZone::reorganizeCards()
{
    if (!cards.isEmpty()) {
        const int cardCount = cards.size();
        if (settingsCache->getHorizontalHand()) {
            bool leftJustified = settingsCache->getLeftJustified();
            qreal cardWidth = cards.at(0)->boundingRect().width();
            const int xPadding = leftJustified ? cardWidth * 1.4 : 5;
            qreal totalWidth = leftJustified? boundingRect().width() - (1 * xPadding) - 5 : boundingRect().width() - 2 * xPadding;
            
            for (int i = 0; i < cardCount; i++) {
                CardItem *c = cards.at(i);
                // If the total width of the cards is smaller than the available width,
                // the cards do not need to overlap and are displayed in the center of the area.
                if (cardWidth * cardCount > totalWidth)
                    c->setPos(xPadding + ((qreal) i) * (totalWidth - cardWidth) / (cardCount - 1), 5);
                else {
                    qreal xPosition = leftJustified ? xPadding + ((qreal) i) * cardWidth : 
                        xPadding + ((qreal) i) * cardWidth + (totalWidth - cardCount * cardWidth) / 2;
                    c->setPos(xPosition, 5);
                }
                c->setRealZValue(i);
            }
        } else {
            qreal totalWidth = boundingRect().width();
            qreal totalHeight = boundingRect().height();
            qreal cardWidth = cards.at(0)->boundingRect().width();
            qreal cardHeight = cards.at(0)->boundingRect().height();
            qreal xspace = 5;
            qreal x1 = xspace;
            qreal x2 = totalWidth - xspace - cardWidth;
        
            for (int i = 0; i < cardCount; i++) {
                CardItem *c = cards.at(i);
                qreal x = i % 2 ? x2 : x1;
                // If the total height of the cards is smaller than the available height,
                // the cards do not need to overlap and are displayed in the center of the area.
                if (cardHeight * cardCount > totalHeight)
                    c->setPos(x, ((qreal) i) * (totalHeight - cardHeight) / (cardCount - 1));
                else
                    c->setPos(x, ((qreal) i) * cardHeight + (totalHeight - cardCount * cardHeight) / 2);
                c->setRealZValue(i);
            }
        }
    }
    update();
}

void HandZone::setWidth(qreal _width)
{
    if (settingsCache->getHorizontalHand()) {
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
