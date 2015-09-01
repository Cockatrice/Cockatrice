#include <QPainter>
#include <QSet>
#include "arrowitem.h"
#include "stackzone.h"
#include "settingscache.h"
#include "thememanager.h"
#include "player.h"
#include "carddragitem.h"
#include "carditem.h"

#include "pb/command_move_card.pb.h"

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

QRectF StackZone::boundingRect() const
{
    return QRectF(0, 0, 100, zoneHeight);
}

void StackZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->fillRect(boundingRect(), themeManager->getStackBgBrush());
}

void StackZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &/*dropPoint*/)
{
    if (startZone == this)
        return;
    
    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(getName().toStdString());
    cmd.set_x(0);
    cmd.set_y(0);
    
    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());

    player->sendGameCommand(cmd);
}

void StackZone::reorganizeCards()
{
    if (!cards.isEmpty()) {
        QList<ArrowItem *> arrowsToUpdate;
        
        const int cardCount = cards.size();
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
            
            arrowsToUpdate.append(c->getArrowsFrom());
            arrowsToUpdate.append(c->getArrowsTo());
        }
        QSetIterator<ArrowItem *> arrowIterator(QSet<ArrowItem *>::fromList(arrowsToUpdate));
        while (arrowIterator.hasNext())
            arrowIterator.next()->updatePath();
    }
    update();
}
