#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include "pilezone.h"
#include "player.h"
#include "carddragitem.h"
#include "zoneviewzone.h"
#include "carditem.h"

#include "pb/command_move_card.pb.h"

PileZone::PileZone(Player *_p, const QString &_name, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent)
    : CardZone(_p, _name, false, _isShufflable, _contentsKnown, parent)
{
    setCacheMode(DeviceCoordinateCache); // Do not move this line to the parent constructor!
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);
    
    setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(90).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));
}

QRectF PileZone::boundingRect() const
{
    return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void PileZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if (!cards.isEmpty())
        cards.at(0)->paintPicture(painter, cards.at(0)->getTranslatedSize(painter), 90);

    painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
    
    painter->translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2);
    painter->rotate(-90);
    painter->translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2);
    paintNumberEllipse(cards.size(), 28, Qt::white, -1, -1, painter);
}

void PileZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
    connect(card, SIGNAL(sigPixmapUpdated()), this, SLOT(callUpdate()));
    cards.insert(x, card);
    card->setPos(0, 0);
    if (!contentsKnown()) {
        card->setName(QString());
        card->setId(-1);
        // If we obscure a previously revealed card, its name has to be forgotten
        if (cards.size() > x + 1)
            cards.at(x + 1)->setName(QString());
    }
    card->setVisible(false);
    card->resetState();
    card->setParentItem(this);
}

void PileZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &/*dropPoint*/)
{
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

void PileZone::reorganizeCards()
{
    update();
}

void PileZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    CardZone::mousePressEvent(event);
    if (event->isAccepted())
        return;

    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else
        event->ignore();
}

void PileZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance())
        return;

    if (cards.isEmpty())
        return;

    bool faceDown = event->modifiers().testFlag(Qt::ShiftModifier);
    bool bottomCard = event->modifiers().testFlag(Qt::ControlModifier);
    CardItem *card = bottomCard ? cards.last() : cards.first();
    const int cardid = contentsKnown() ? card->getId() : (bottomCard ? cards.size() - 1 : 0);
    CardDragItem *drag = card->createDragItem(cardid, event->pos(), event->scenePos(), faceDown);
    drag->grabMouse();
    setCursor(Qt::OpenHandCursor);
}

void PileZone::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    setCursor(Qt::OpenHandCursor);
}

void PileZone::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!cards.isEmpty())
        cards[0]->processHoverEvent();
    QGraphicsItem::hoverEnterEvent(event);
}
