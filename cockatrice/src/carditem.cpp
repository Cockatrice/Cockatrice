#include "carditem.h"

#include "arrowitem.h"
#include "carddatabase.h"
#include "carddragitem.h"
#include "cardzone.h"
#include "gamescene.h"
#include "main.h"
#include "pb/serverinfo_card.pb.h"
#include "player.h"
#include "settingscache.h"
#include "tab_game.h"
#include "tablezone.h"
#include "zoneviewzone.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

CardItem::CardItem(Player *_owner,
                   const QString &_name,
                   int _cardid,
                   bool _revealedCard,
                   QGraphicsItem *parent,
                   CardZone *_zone)
    : AbstractCardItem(_name, _owner, _cardid, parent), zone(_zone), revealedCard(_revealedCard), attacking(false),
      destroyOnZoneChange(false), doesntUntap(false), dragItem(nullptr), attachedTo(nullptr)
{
    owner->addCard(this);

    cardMenu = new QMenu;
    ptMenu = new QMenu;
    moveMenu = new QMenu;

    retranslateUi();
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
    if (owner != nullptr) {
        if (owner->getCardMenu() == cardMenu) {
            owner->setCardMenu(nullptr);
            owner->getGame()->setActiveCard(nullptr);
        }
        owner = nullptr;
    }

    while (!attachedCards.isEmpty()) {
        attachedCards.first()->setZone(nullptr); // so that it won't try to call reorganizeCards()
        attachedCards.first()->setAttachedTo(nullptr);
    }

    if (attachedTo != nullptr) {
        attachedTo->removeAttachedCard(this);
        attachedTo = nullptr;
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
        QColor _color;
        _color.setHsv(counterIterator.key() * 60, 150, 255);

        paintNumberEllipse(counterIterator.value(), 14, _color, i, counters.size(), painter);
        ++i;
    }

    QSizeF translatedSize = getTranslatedSize(painter);
    qreal scaleFactor = translatedSize.width() / boundingRect().width();

    if (!pt.isEmpty()) {
        painter->save();
        transformPainter(painter, translatedSize, tapAngle);

        if (!getFaceDown() && info && pt == info->getPowTough()) {
            painter->setPen(Qt::white);
        } else {
            painter->setPen(QColor(255, 150, 0)); // dark orange
        }

        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);

        painter->drawText(QRectF(4 * scaleFactor, 4 * scaleFactor, translatedSize.width() - 10 * scaleFactor,
                                 translatedSize.height() - 8 * scaleFactor),
                          Qt::AlignRight | Qt::AlignBottom, pt);
        painter->restore();
    }

    if (!annotation.isEmpty()) {
        painter->save();

        transformPainter(painter, translatedSize, tapAngle);
        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setPen(Qt::white);

        painter->drawText(QRectF(4 * scaleFactor, 4 * scaleFactor, translatedSize.width() - 8 * scaleFactor,
                                 translatedSize.height() - 8 * scaleFactor),
                          Qt::AlignCenter | Qt::TextWrapAnywhere, annotation);
        painter->restore();
    }

    if (getBeingPointedAt()) {
        painter->fillPath(shape(), QBrush(QColor(255, 0, 0, 100)));
    }

    if (doesntUntap) {
        painter->save();

        painter->setRenderHint(QPainter::Antialiasing, false);

        QPen pen;
        pen.setColor(Qt::magenta);
        pen.setWidth(0); // Cosmetic pen
        painter->setPen(pen);
        painter->drawPath(shape());

        painter->restore();
    }

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
    update();
}

void CardItem::setPT(const QString &_pt)
{
    pt = _pt;
    update();
}

void CardItem::setAttachedTo(CardItem *_attachedTo)
{
    if (attachedTo != nullptr) {
        attachedTo->removeAttachedCard(this);
    }

    gridPoint.setX(-1);
    attachedTo = _attachedTo;
    if (attachedTo != nullptr) {
        setParentItem(attachedTo->getZone());
        attachedTo->addAttachedCard(this);
        if (zone != attachedTo->getZone()) {
            attachedTo->getZone()->reorganizeCards();
        }
    } else {
        setParentItem(zone);
    }

    if (zone != nullptr) {
        zone->reorganizeCards();
    }
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

void CardItem::processCardInfo(const ServerInfo_Card &_info)
{
    counters.clear();
    const int counterListSize = _info.counter_list_size();
    for (int i = 0; i < counterListSize; ++i) {
        const ServerInfo_CardCounter &counterInfo = _info.counter_list(i);
        counters.insert(counterInfo.id(), counterInfo.value());
    }

    setId(_info.id());
    setName(QString::fromStdString(_info.name()));
    setAttacking(_info.attacking());
    setFaceDown(_info.face_down());
    setPT(QString::fromStdString(_info.pt()));
    setAnnotation(QString::fromStdString(_info.annotation()));
    setColor(QString::fromStdString(_info.color()));
    setTapped(_info.tapped());
    setDestroyOnZoneChange(_info.destroy_on_zone_change());
    setDoesntUntap(_info.doesnt_untap());
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
    if (dragItem) {
        dragItem->deleteLater();
    }
    dragItem = nullptr;
}

void CardItem::drawArrow(const QColor &arrowColor)
{
    if (static_cast<TabGame *>(owner->parent())->getSpectator())
        return;

    Player *arrowOwner = static_cast<TabGame *>(owner->parent())->getActiveLocalPlayer();
    ArrowDragItem *arrow = new ArrowDragItem(arrowOwner, this, arrowColor);
    scene()->addItem(arrow);
    arrow->grabMouse();

    for (const auto &item : scene()->selectedItems()) {
        CardItem *card = qgraphicsitem_cast<CardItem *>(item);
        if (card == nullptr || card == this)
            continue;
        if (card->getZone() != zone)
            continue;

        ArrowDragItem *childArrow = new ArrowDragItem(arrowOwner, card, arrowColor);
        scene()->addItem(childArrow);
        arrow->addChildArrow(childArrow);
    }
}

void CardItem::drawAttachArrow()
{
    if (static_cast<TabGame *>(owner->parent())->getSpectator())
        return;

    auto *arrow = new ArrowAttachItem(this);
    scene()->addItem(arrow);
    arrow->grabMouse();

    for (const auto &item : scene()->selectedItems()) {
        CardItem *card = qgraphicsitem_cast<CardItem *>(item);
        if (card == nullptr)
            continue;
        if (card->getZone() != zone)
            continue;

        ArrowAttachItem *childArrow = new ArrowAttachItem(card);
        scene()->addItem(childArrow);
        arrow->addChildArrow(childArrow);
    }
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::RightButton)) {
        if ((event->screenPos() - event->buttonDownScreenPos(Qt::RightButton)).manhattanLength() <
            2 * QApplication::startDragDistance())
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
        if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() <
            2 * QApplication::startDragDistance())
            return;
        if (zone->getIsView()) {
            const ZoneViewZone *view = static_cast<const ZoneViewZone *>(zone);
            if (view->getRevealZone() && !view->getWriteableRevealZone())
                return;
        } else if (!owner->getLocalOrJudge())
            return;

        bool forceFaceDown = event->modifiers().testFlag(Qt::ShiftModifier);

        createDragItem(id, event->pos(), event->scenePos(), facedown || forceFaceDown);
        dragItem->grabMouse();

        int childIndex = 0;
        for (const auto &item : scene()->selectedItems()) {
            CardItem *card = static_cast<CardItem *>(item);
            if ((card == this) || (card->getZone() != zone))
                continue;
            ++childIndex;
            QPointF childPos;
            if (zone->getHasCardAttr())
                childPos = card->pos() - pos();
            else
                childPos = QPointF(childIndex * CARD_WIDTH / 2, 0);
            CardDragItem *drag =
                new CardDragItem(card, card->getId(), childPos, card->getFaceDown() || forceFaceDown, dragItem);
            drag->setPos(dragItem->pos() + childPos);
            scene()->addItem(drag);
        }
    }
    setCursor(Qt::OpenHandCursor);
}

void CardItem::playCard(bool faceDown)
{
    // Do nothing if the card belongs to another player
    if (!owner->getLocalOrJudge())
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
        if (cardMenu != nullptr && !cardMenu->isEmpty() && owner != nullptr) {
            cardMenu->popup(event->screenPos());
            return;
        }
    } else if ((event->modifiers() != Qt::AltModifier) && (event->button() == Qt::LeftButton) &&
               (!SettingsCache::instance().getDoubleClickToPlay())) {
        bool hideCard = false;
        if (zone && zone->getIsView()) {
            auto *view = static_cast<ZoneViewZone *>(zone);
            if (view->getRevealZone() && !view->getWriteableRevealZone())
                hideCard = true;
        }
        if (zone && hideCard) {
            zone->removeCard(this);
        } else {
            playCard(event->modifiers().testFlag(Qt::ShiftModifier));
        }
    }

    if (owner != nullptr) { // cards without owner will be deleted
        setCursor(Qt::OpenHandCursor);
    }
    AbstractCardItem::mouseReleaseEvent(event);
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->modifiers() != Qt::AltModifier) && (SettingsCache::instance().getDoubleClickToPlay()) &&
        (event->buttons() == Qt::LeftButton)) {
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
    bool animationIncomplete = true;
    if (!tapped)
        rotation *= -1;

    tapAngle += rotation;
    if (tapped && (tapAngle > 90)) {
        tapAngle = 90;
        animationIncomplete = false;
    }
    if (!tapped && (tapAngle < 0)) {
        tapAngle = 0;
        animationIncomplete = false;
    }

    setTransform(QTransform()
                     .translate(CARD_WIDTH_HALF, CARD_HEIGHT_HALF)
                     .rotate(tapAngle)
                     .translate(-CARD_WIDTH_HALF, -CARD_HEIGHT_HALF));
    setHovered(false);
    update();

    return animationIncomplete;
}

QVariant CardItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == ItemSelectedHasChanged) && owner != nullptr) {
        if (value == true) {
            owner->setCardMenu(cardMenu);
            owner->getGame()->setActiveCard(this);
        } else if (owner->getCardMenu() == cardMenu) {
            owner->setCardMenu(nullptr);
            owner->getGame()->setActiveCard(nullptr);
        }
    }
    return QGraphicsItem::itemChange(change, value);
}
