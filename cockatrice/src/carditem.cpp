#include <QApplication>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include "gamescene.h"
#include "carditem.h"
#include "carddragitem.h"
#include "carddatabase.h"
#include "cardzone.h"
#include "zoneviewzone.h"
#include "tablezone.h"
#include "player.h"
#include "arrowitem.h"
#include "main.h"
#include "settingscache.h"
#include "tab_game.h"
#include "pb/serverinfo_card.pb.h"


CardItem::CardItem(Player *_owner, const QString &_name, int _cardid, bool _revealedCard, QGraphicsItem *parent)
    : AbstractCardItem(_name, _owner, _cardid, parent), zone(0), revealedCard(_revealedCard), attacking(false), destroyOnZoneChange(false), doesntUntap(false), dragItem(0), attachedTo(0)
{
    owner->addCard(this);
    
    cardMenu = new QMenu;
    ptMenu = new QMenu;
    moveMenu = new QMenu;
    
    retranslateUi();
    emit updateCardMenu(this);
}

CardItem::~CardItem()
{
    prepareDelete();
    
    if (scene())
        static_cast<GameScene *>(scene())->unregisterAnimationItem(this);
    
    delete cardMenu;
    delete ptMenu;
    delete moveMenu;
    
    deleteDragItem();
}

void CardItem::prepareDelete()
{
    if (owner) {
        if (owner->getCardMenu() == cardMenu) {
            owner->setCardMenu(0);
            owner->getGame()->setActiveCard(0);
        }
        owner = 0;
    }
    
    while (!attachedCards.isEmpty()) {
        attachedCards.first()->setZone(0); // so that it won't try to call reorganizeCards()
        attachedCards.first()->setAttachedTo(0);
    }
    
    if (attachedTo) {
        attachedTo->removeAttachedCard(this);
        attachedTo = 0;
    }
}

void CardItem::deleteLater()
{
    prepareDelete();
    AbstractCardItem::deleteLater();
}

void CardItem::setZone(CardZone *_zone)
{
    zone = _zone;
    emit updateCardMenu(this);
}

void CardItem::retranslateUi()
{
    moveMenu->setTitle(tr("&Move to"));
    ptMenu->setTitle(tr("&Power / toughness"));
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    AbstractCardItem::paint(painter, option, widget);
    
    int i = 0;
    QMapIterator<int, int> counterIterator(counters);
    while (counterIterator.hasNext()) {
        counterIterator.next();
        QColor color;
        color.setHsv(counterIterator.key() * 60, 150, 255);
        
        paintNumberEllipse(counterIterator.value(), 14, color, i, counters.size(), painter);
        ++i;
    }
    
    QSizeF translatedSize = getTranslatedSize(painter);
    qreal scaleFactor = translatedSize.width() / boundingRect().width();
    
    if (!pt.isEmpty()) {
        painter->save();
        transformPainter(painter, translatedSize, tapAngle);

        if(info)
        {
            QStringList ptSplit = pt.split("/");
            QStringList ptDbSplit = info->getPowTough().split("/");
        
            if (getFaceDown() || ptDbSplit.at(0) != ptSplit.at(0) || ptDbSplit.at(1) != ptSplit.at(1))
                painter->setPen(QColor(255, 150, 0));
            else
                painter->setPen(Qt::white);
        } else {
            painter->setPen(Qt::white);
        }
        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);
        
        painter->drawText(QRectF(4 * scaleFactor, 4 * scaleFactor, translatedSize.width() - 10 * scaleFactor, translatedSize.height() - 8 * scaleFactor), Qt::AlignRight | Qt::AlignBottom, pt);
        painter->restore();
    }
    if (!annotation.isEmpty()) {
        painter->save();

        transformPainter(painter, translatedSize, tapAngle);
        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setPen(Qt::white);
        
        painter->drawText(QRectF(4 * scaleFactor, 4 * scaleFactor, translatedSize.width() - 8 * scaleFactor, translatedSize.height() - 8 * scaleFactor), Qt::AlignCenter | Qt::TextWrapAnywhere, annotation);
        painter->restore();
    }
    if (getBeingPointedAt())
        painter->fillRect(boundingRect(), QBrush(QColor(255, 0, 0, 100)));
    painter->restore();
}

void CardItem::setAttacking(bool _attacking)
{
    attacking = _attacking;
    update();
}

void CardItem::setCounter(int _id, int _value)
{
    if (_value)
        counters.insert(_id, _value);
    else
        counters.remove(_id);
    update();
}

void CardItem::setAnnotation(const QString &_annotation)
{
    annotation = _annotation;
    update();
}

void CardItem::setDoesntUntap(bool _doesntUntap)
{
    doesntUntap = _doesntUntap;
}

void CardItem::setPT(const QString &_pt)
{
    pt = _pt;
    update();
}

void CardItem::setAttachedTo(CardItem *_attachedTo)
{
    if (attachedTo)
        attachedTo->removeAttachedCard(this);
    
    gridPoint.setX(-1);
    attachedTo = _attachedTo;
    if (attachedTo) {
        setParentItem(attachedTo->getZone());
        attachedTo->addAttachedCard(this);
        if (zone != attachedTo->getZone())
            attachedTo->getZone()->reorganizeCards();
    } else
        setParentItem(zone);

    if (zone)
        zone->reorganizeCards();
    
    emit updateCardMenu(this);
}

void CardItem::resetState()
{
    attacking = false;
    facedown = false;
    counters.clear();
    pt.clear();
    annotation.clear();
    attachedTo = 0;
    attachedCards.clear();
    setTapped(false, false);
    setDoesntUntap(false);
    if (scene())
        static_cast<GameScene *>(scene())->unregisterAnimationItem(this);
    update();
}

void CardItem::processCardInfo(const ServerInfo_Card &info)
{
    counters.clear();
    const int counterListSize = info.counter_list_size();
    for (int i = 0; i < counterListSize; ++i) {
        const ServerInfo_CardCounter &counterInfo = info.counter_list(i);
        counters.insert(counterInfo.id(), counterInfo.value());
    }
    
    setId(info.id());
    setName(QString::fromStdString(info.name()));
    setAttacking(info.attacking());
    setFaceDown(info.face_down());
    setPT(QString::fromStdString(info.pt()));
    setAnnotation(QString::fromStdString(info.annotation()));
    setColor(QString::fromStdString(info.color()));
    setTapped(info.tapped());
    setDestroyOnZoneChange(info.destroy_on_zone_change());
    setDoesntUntap(info.doesnt_untap());
}

CardDragItem *CardItem::createDragItem(int _id, const QPointF &_pos, const QPointF &_scenePos, bool faceDown)
{
    deleteDragItem();
    dragItem = new CardDragItem(this, _id, _pos, faceDown);
    dragItem->setVisible(false);
    scene()->addItem(dragItem);
    dragItem->updatePosition(_scenePos);
    dragItem->setVisible(true);

    return dragItem;
}

void CardItem::deleteDragItem()
{
    if(dragItem)
        dragItem->deleteLater();
    dragItem = NULL;
}

void CardItem::drawArrow(const QColor &arrowColor)
{
    if (static_cast<TabGame *>(owner->parent())->getSpectator())
        return;
    
    Player *arrowOwner = static_cast<TabGame *>(owner->parent())->getActiveLocalPlayer();
    ArrowDragItem *arrow = new ArrowDragItem(arrowOwner, this, arrowColor);
    scene()->addItem(arrow);
    arrow->grabMouse();
    
    QListIterator<QGraphicsItem *> itemIterator(scene()->selectedItems());
    while (itemIterator.hasNext()) {
        CardItem *c = qgraphicsitem_cast<CardItem *>(itemIterator.next());
        if (!c || (c == this))
            continue;
        if (c->getZone() != zone)
            continue;
        
        ArrowDragItem *childArrow = new ArrowDragItem(arrowOwner, c, arrowColor);
        scene()->addItem(childArrow);
        arrow->addChildArrow(childArrow);
    }
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::RightButton)) {
        if ((event->screenPos() - event->buttonDownScreenPos(Qt::RightButton)).manhattanLength() < 2 * QApplication::startDragDistance())
            return;
        
        QColor arrowColor = Qt::red;
        if (event->modifiers().testFlag(Qt::ControlModifier))
            arrowColor = Qt::yellow;
        else if (event->modifiers().testFlag(Qt::AltModifier))
            arrowColor = Qt::blue;
        else if (event->modifiers().testFlag(Qt::ShiftModifier))
            arrowColor = Qt::green;
        
        drawArrow(arrowColor);
    } else if (event->buttons().testFlag(Qt::LeftButton)) {
        if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < 2 * QApplication::startDragDistance())
            return;
        if (zone->getIsView()) {
            const ZoneViewZone *const view = static_cast<const ZoneViewZone *const>(zone);
            if (view->getRevealZone() && !view->getWriteableRevealZone())
                return;
        } else if (!owner->getLocal())
            return;
        
        bool forceFaceDown = event->modifiers().testFlag(Qt::ShiftModifier);
    
        createDragItem(id, event->pos(), event->scenePos(), facedown || forceFaceDown);
        dragItem->grabMouse();
        
        QList<QGraphicsItem *> sel = scene()->selectedItems();
        int j = 0;
        for (int i = 0; i < sel.size(); i++) {
            CardItem *c = (CardItem *) sel.at(i);
            if ((c == this) || (c->getZone() != zone))
                continue;
            ++j;
            QPointF childPos;
            if (zone->getHasCardAttr())
                childPos = c->pos() - pos();
            else
                childPos = QPointF(j * CARD_WIDTH / 2, 0);
            CardDragItem *drag = new CardDragItem(c, c->getId(), childPos, c->getFaceDown() || forceFaceDown, dragItem);
            drag->setPos(dragItem->pos() + childPos);
            scene()->addItem(drag);
        }
    }
    setCursor(Qt::OpenHandCursor);
}

void CardItem::playCard(bool faceDown)
{
    // Do nothing if the card belongs to another player
    if (!owner->getLocal())
        return;

    TableZone *tz = qobject_cast<TableZone *>(zone);
    if (tz)
        tz->toggleTapped();
    else
        zone->getPlayer()->playCard(this, faceDown, info ? info->getCipt() : false);
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (cardMenu)
            if (!cardMenu->isEmpty())
                cardMenu->exec(event->screenPos());
    } else if ((event->button() == Qt::LeftButton) && !settingsCache->getDoubleClickToPlay()) {
        
        bool hideCard = false;
        if (zone->getIsView()) {
            ZoneViewZone *view = static_cast<ZoneViewZone *>(zone);
            if (view->getRevealZone() && !view->getWriteableRevealZone())
                hideCard = true;
        }
        if (hideCard)
            zone->removeCard(this);
        else
            playCard(event->modifiers().testFlag(Qt::ShiftModifier));
    }

    setCursor(Qt::OpenHandCursor);
    AbstractCardItem::mouseReleaseEvent(event);
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (settingsCache->getDoubleClickToPlay() && event->buttons() == Qt::LeftButton) {
        if (revealedCard)
            zone->removeCard(this);
        else
            playCard(event->modifiers().testFlag(Qt::ShiftModifier));
    }
    event->accept();
}


bool CardItem::animationEvent()
{
    int rotation = ROTATION_DEGREES_PER_FRAME;
    if (!tapped)
        rotation *= -1;

    tapAngle += rotation;

    setTransform(QTransform().translate(CARD_WIDTH_HALF, CARD_HEIGHT_HALF).rotate(tapAngle).translate(-CARD_WIDTH_HALF, -CARD_HEIGHT_HALF));
    setHovered(false);
    update();

    if ((tapped && (tapAngle >= 90)) || (!tapped && (tapAngle <= 0)))
        return false;
    return true;
}

QVariant CardItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == ItemSelectedHasChanged) && owner) {
        if (value == true) {
            owner->setCardMenu(cardMenu);
            owner->getGame()->setActiveCard(this);
        } else if (owner->getCardMenu() == cardMenu) {
            owner->setCardMenu(0);
            owner->getGame()->setActiveCard(0);
        }
    }
    return QGraphicsItem::itemChange(change, value);
}
