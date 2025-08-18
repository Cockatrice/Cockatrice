#include "card_item.h"

#include "../../client/tabs/tab_game.h"
#include "../../settings/cache_settings.h"
#include "../../settings/card_counter_settings.h"
#include "../cards/card_info.h"
#include "../game_scene.h"
#include "../player/player.h"
#include "../zones/card_zone.h"
#include "../zones/table_zone.h"
#include "../zones/view_zone.h"
#include "arrow_item.h"
#include "card_drag_item.h"
#include "pb/serverinfo_card.pb.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

CardItem::CardItem(Player *_owner, QGraphicsItem *parent, const CardRef &cardRef, int _cardid, CardZone *_zone)
    : AbstractCardItem(parent, cardRef, _owner, _cardid), zone(_zone), attacking(false), destroyOnZoneChange(false),
      doesntUntap(false), dragItem(nullptr), attachedTo(nullptr)
{
    owner->addCard(this);

    connect(&SettingsCache::instance().cardCounters(), &CardCounterSettings::colorChanged, this, [this](int counterId) {
        if (counters.contains(counterId))
            update();
    });
}

void CardItem::prepareDelete()
{
    if (owner != nullptr) {
        if (owner->getGame()->getActiveCard() == this) {
            owner->updateCardMenu(nullptr);
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
    if (scene())
        static_cast<GameScene *>(scene())->unregisterAnimationItem(this);
    AbstractCardItem::deleteLater();
}

void CardItem::setZone(CardZone *_zone)
{
    zone = _zone;
}

void CardItem::retranslateUi()
{
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    auto &cardCounterSettings = SettingsCache::instance().cardCounters();

    painter->save();
    AbstractCardItem::paint(painter, option, widget);

    int i = 0;
    QMapIterator<int, int> counterIterator(counters);
    while (counterIterator.hasNext()) {
        counterIterator.next();
        QColor _color = cardCounterSettings.color(counterIterator.key());

        paintNumberEllipse(counterIterator.value(), 14, _color, i, counters.size(), painter);
        ++i;
    }

    QSizeF translatedSize = getTranslatedSize(painter);
    qreal scaleFactor = translatedSize.width() / boundingRect().width();

    if (!pt.isEmpty()) {
        painter->save();
        transformPainter(painter, translatedSize, tapAngle);

        if (!getFaceDown() && pt == exactCard.getInfo().getPowTough()) {
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

void CardItem::resetState(bool keepAnnotations)
{
    attacking = false;
    facedown = false;
    counters.clear();
    pt.clear();
    if (!keepAnnotations) {
        annotation.clear();
    }
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
    setCardRef({QString::fromStdString(_info.name()), QString::fromStdString(_info.provider_id())});
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
    if (static_cast<TabGame *>(owner->parent())->isSpectator())
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
    if (static_cast<TabGame *>(owner->parent())->isSpectator())
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
        if (const ZoneViewZone *view = qobject_cast<const ZoneViewZone *>(zone)) {
            if (view->getRevealZone() && !view->getWriteableRevealZone())
                return;
        } else if (!owner->getLocalOrJudge())
            return;

        bool forceFaceDown = event->modifiers().testFlag(Qt::ShiftModifier);

        // Use the buttonDownPos to align the hot spot with the position when
        // the user originally clicked
        createDragItem(id, event->buttonDownPos(Qt::LeftButton), event->scenePos(), facedown || forceFaceDown);
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
    else {
        if (SettingsCache::instance().getClickPlaysAllSelected()) {
            faceDown ? zone->getPlayer()->actPlayFacedown() : zone->getPlayer()->actPlay();
        } else {
            zone->getPlayer()->playCard(this, faceDown);
        }
    }
}

/**
 * @brief returns true if the zone is a unwritable reveal zone view (eg a card reveal window). Will return false if zone
 * is nullptr.
 */
static bool isUnwritableRevealZone(CardZone *zone)
{
    if (auto *view = qobject_cast<ZoneViewZone *>(zone)) {
        return view->getRevealZone() && !view->getWriteableRevealZone();
    }
    return false;
}

/**
 * This method is called when a "click to play" is done on the card.
 * This is either triggered by a single click or double click, depending on the settings.
 *
 * @param shiftHeld if the shift key was held during the click
 */
void CardItem::handleClickedToPlay(bool shiftHeld)
{
    if (isUnwritableRevealZone(zone)) {
        if (SettingsCache::instance().getClickPlaysAllSelected()) {
            zone->getPlayer()->actHide();
        } else {
            zone->removeCard(this);
        }
    } else {
        playCard(shiftHeld);
    }
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {

        if (owner != nullptr) {
            owner->getGame()->setActiveCard(this);
            if (QMenu *cardMenu = owner->updateCardMenu(this)) {
                cardMenu->popup(event->screenPos());
                return;
            }
        }
    } else if ((event->modifiers() != Qt::AltModifier) && (event->button() == Qt::LeftButton) &&
               (!SettingsCache::instance().getDoubleClickToPlay())) {
        handleClickedToPlay(event->modifiers().testFlag(Qt::ShiftModifier));
    }

    if (owner != nullptr) { // cards without owner will be deleted
        setCursor(Qt::OpenHandCursor);
    }
    AbstractCardItem::mouseReleaseEvent(event);
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->modifiers() != Qt::AltModifier) && (event->buttons() == Qt::LeftButton) &&
        (SettingsCache::instance().getDoubleClickToPlay())) {
        handleClickedToPlay(event->modifiers().testFlag(Qt::ShiftModifier));
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
            owner->getGame()->setActiveCard(this);
            owner->updateCardMenu(this);
        } else if (owner->scene()->selectedItems().isEmpty()) {
            owner->getGame()->setActiveCard(nullptr);
            owner->updateCardMenu(nullptr);
        }
    }
    return AbstractCardItem::itemChange(change, value);
}
