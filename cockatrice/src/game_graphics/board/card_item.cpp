#include "card_item.h"

#include "../../client/settings/cache_settings.h"
#include "../../game/phase.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../../game/zones/view_zone_logic.h"
#include "../../interface/widgets/tabs/tab_game.h"
#include "../game_scene.h"
#include "../zones/table_zone.h"
#include "../zones/view_zone.h"
#include "arrow_item.h"
#include "card_drag_item.h"

#include <../../client/settings/card_counter_settings.h>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>

CardItem::CardItem(CardState *_state, CardZone *parent)
    : AbstractCardItem(_state, parent), state(_state), zone(parent), dragItem(nullptr)
{
    connect(&SettingsCache::instance().cardCounters(), &CardCounterSettings::colorChanged, this, [this](int counterId) {
        if (state->getCounters().contains(counterId)) {
            update();
        }
    });

    connect(state, &CardState::visibleChanged, this, [this](bool visible) { setVisible(visible); });
}

void CardItem::prepareDelete()
{
    if (owner != nullptr) {
        if (owner->getGame()->getActiveCard() == this) {
            emit owner->requestCardMenuUpdate(nullptr);
            owner->getGame()->setActiveCard(nullptr);
        }
        owner = nullptr;
    }

    state->prepareDelete();

    if (state->getAttachedTo() != nullptr) {
        state->getAttachedTo()->removeAttachedCard(getState());
        state->setAttachedTo(nullptr);
    }
}

void CardItem::deleteLater()
{
    prepareDelete();
    if (scene()) {
        static_cast<GameScene *>(scene())->unregisterAnimationItem(this);
    }
    AbstractCardItem::deleteLater();
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
    QMapIterator<int, int> counterIterator(state->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next();
        QColor _color = cardCounterSettings.color(counterIterator.key());

        paintNumberEllipse(counterIterator.value(), 14, _color, i, state->getCounters().size(), painter);
        ++i;
    }

    QSizeF translatedSize = getTranslatedSize(painter);
    qreal scaleFactor = translatedSize.width() / boundingRect().width();

    if (!state->getPT().isEmpty()) {
        painter->save();
        transformPainter(painter, translatedSize, tapAngle);

        if (!getState()->getFaceDown() && state->getPT() == getState()->getCard().getInfo().getPowTough()) {
            painter->setPen(Qt::white);
        } else {
            painter->setPen(QColor(255, 150, 0)); // dark orange
        }

        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);

        painter->drawText(QRectF(4 * scaleFactor, 4 * scaleFactor, translatedSize.width() - 10 * scaleFactor,
                                 translatedSize.height() - 8 * scaleFactor),
                          Qt::AlignRight | Qt::AlignBottom, state->getPT());
        painter->restore();
    }

    if (!state->getAnnotation().isEmpty()) {
        painter->save();

        transformPainter(painter, translatedSize, tapAngle);
        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setPen(Qt::white);

        painter->drawText(QRectF(4 * scaleFactor, 4 * scaleFactor, translatedSize.width() - 8 * scaleFactor,
                                 translatedSize.height() - 8 * scaleFactor),
                          Qt::AlignCenter | Qt::TextWrapAnywhere, state->getAnnotation());
        painter->restore();
    }

    if (getBeingPointedAt()) {
        painter->fillPath(shape(), QBrush(QColor(255, 0, 0, 100)));
    }

    if (state->getDoesntUntap()) {
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

/**
 * @brief Resets the fields that should be reset after a zone transition
 */
void CardItem::resetState(bool keepAnnotations)
{
    state->resetState(keepAnnotations);
    state->clearAttachedCards();
    state->setTapped(false, false);
    getState()->setDoesntUntap(false);
    if (scene()) {
        static_cast<GameScene *>(scene())->unregisterAnimationItem(this);
    }
    update();
}

CardDragItem *CardItem::createDragItem(int _id, const QPointF &_pos, const QPointF &_scenePos, bool forceFaceDown)
{
    deleteDragItem();
    dragItem = new CardDragItem(this, _id, _pos, forceFaceDown);
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
    if (owner->getGame()->getPlayerManager()->isSpectator()) {
        return;
    }

    auto *game = owner->getGame();
    PlayerLogic *arrowOwner = game->getPlayerManager()->getActiveLocalPlayer(game->getGameState()->getActivePlayer());
    int phase = 0; // 0 means to not set the phase
    if (SettingsCache::instance().getDoNotDeleteArrowsInSubPhases()) {
        int currentPhase = game->getGameState()->getCurrentPhase();
        phase = Phases::getLastSubphase(currentPhase) + 1;
    }
    ArrowDragItem *arrow = new ArrowDragItem(arrowOwner, this, arrowColor, phase);
    scene()->addItem(arrow);
    arrow->grabMouse();

    for (const auto &item : scene()->selectedItems()) {
        CardItem *card = qgraphicsitem_cast<CardItem *>(item);
        if (card == nullptr || card == this) {
            continue;
        }
        if (card->getState()->getZone() != state->getZone()) {
            continue;
        }

        ArrowDragItem *childArrow = new ArrowDragItem(arrowOwner, card, arrowColor, phase);
        scene()->addItem(childArrow);
        arrow->addChildArrow(childArrow);
    }
}

void CardItem::drawAttachArrow()
{
    if (owner->getGame()->getPlayerManager()->isSpectator()) {
        return;
    }

    auto *arrow = new ArrowAttachItem(this);
    scene()->addItem(arrow);
    arrow->grabMouse();

    for (const auto &item : scene()->selectedItems()) {
        CardItem *card = qgraphicsitem_cast<CardItem *>(item);
        if (card == nullptr) {
            continue;
        }
        if (card->getState()->getZone() != state->getZone()) {
            continue;
        }

        ArrowAttachItem *childArrow = new ArrowAttachItem(card);
        scene()->addItem(childArrow);
        arrow->addChildArrow(childArrow);
    }
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::RightButton)) {
        if ((event->screenPos() - event->buttonDownScreenPos(Qt::RightButton)).manhattanLength() <
            2 * QApplication::startDragDistance()) {
            return;
        }

        QColor arrowColor = Qt::red;
        if (event->modifiers().testFlag(Qt::ControlModifier)) {
            arrowColor = Qt::yellow;
        } else if (event->modifiers().testFlag(Qt::AltModifier)) {
            arrowColor = Qt::blue;
        } else if (event->modifiers().testFlag(Qt::ShiftModifier)) {
            arrowColor = Qt::green;
        }

        drawArrow(arrowColor);
    } else if (event->buttons().testFlag(Qt::LeftButton)) {
        if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() <
            2 * QApplication::startDragDistance()) {
            return;
        }
        if (const ZoneViewZoneLogic *view = qobject_cast<const ZoneViewZoneLogic *>(state->getZone())) {
            if (view->getRevealZone() && !view->getWriteableRevealZone()) {
                return;
            }
        } else if (!owner->getPlayerInfo()->getLocalOrJudge()) {
            return;
        }

        bool forceFaceDown = event->modifiers().testFlag(Qt::ShiftModifier);

        // Use the buttonDownPos to align the hot spot with the position when
        // the user originally clicked
        createDragItem(getState()->getId(), event->buttonDownPos(Qt::LeftButton), event->scenePos(), forceFaceDown);
        dragItem->grabMouse();

        int childIndex = 0;
        for (const auto &item : scene()->selectedItems()) {
            CardItem *card = static_cast<CardItem *>(item);
            if ((card == this) || (card->getState()->getZone() != state->getZone())) {
                continue;
            }
            ++childIndex;
            QPointF childPos;
            if (state->getZone()->getHasCardAttr()) {
                childPos = card->pos() - pos();
            } else {
                childPos = QPointF(childIndex * CardDimensions::WIDTH_HALF_F, 0);
            }
            CardDragItem *drag = new CardDragItem(card, card->getState()->getId(), childPos,
                                                  card->getState()->getFaceDown() || forceFaceDown, dragItem);
            drag->setPos(dragItem->pos() + childPos);
            scene()->addItem(drag);
        }
    }
    setCursor(Qt::OpenHandCursor);
}

void CardItem::playCard(bool faceDown)
{
    // Do nothing if the card belongs to another player
    if (!owner->getPlayerInfo()->getLocalOrJudge()) {
        return;
    }

    TableZoneLogic *tz = qobject_cast<TableZoneLogic *>(state->getZone());
    if (tz) {
        emit tz->toggleTapped();
    } else {
        if (SettingsCache::instance().getClickPlaysAllSelected()) {
            if (faceDown) {
                emit playSelectedFaceDown(this);
            } else {
                emit playSelected(this);
            }
        } else {
            state->getZone()->getPlayer()->getPlayerActions()->playCard(getState(), faceDown);
        }
    }
}

QVariantList CardItem::parsePT(const QString &pt)
{
    QVariantList ptList = QVariantList();
    if (!pt.isEmpty()) {
        int sep = pt.indexOf('/');
        if (sep == 0) {
            ptList.append(QVariant(pt.mid(1))); // cut off starting '/' and take full string
        } else {
            int start = 0;
            for (;;) {
                QString item = pt.mid(start, sep - start);
                if (item.isEmpty()) {
                    ptList.append(QVariant(QString()));
                } else if (item[0] == '+') {
                    ptList.append(QVariant(item.mid(1).toInt())); // add as int
                } else if (item[0] == '-') {
                    ptList.append(QVariant(item.toInt())); // add as int
                } else {
                    ptList.append(QVariant(item)); // add as qstring
                }
                if (sep == -1) {
                    break;
                }
                start = sep + 1;
                sep = pt.indexOf('/', start);
            }
        }
    }
    return ptList;
}

/**
 * @brief returns true if the zone is a unwritable reveal zone view (eg a card reveal window). Will return false if zone
 * is nullptr.
 */
static bool isUnwritableRevealZone(CardZoneLogic *zone)
{
    if (auto *view = qobject_cast<ZoneViewZoneLogic *>(zone)) {
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
    if (isUnwritableRevealZone(state->getZone())) {
        if (SettingsCache::instance().getClickPlaysAllSelected()) {
            emit hideSelected(this);
        } else {
            state->getZone()->removeCard(getState());
        }
    } else {
        playCard(shiftHeld);
    }
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton && owner != nullptr) {
        emit rightClicked(this, event->screenPos());
    }
    if ((event->modifiers() != Qt::AltModifier) && (event->button() == Qt::LeftButton) &&
        (!SettingsCache::instance().getDoubleClickToPlay())) {
        handleClickedToPlay(event->modifiers().testFlag(Qt::ShiftModifier));
    }
    if (owner != nullptr) {
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
    if (!getState()->getTapped()) {
        rotation *= -1;
    }

    tapAngle += rotation;
    if (getState()->getTapped() && (tapAngle > 90)) {
        tapAngle = 90;
        animationIncomplete = false;
    }
    if (!getState()->getTapped() && (tapAngle < 0)) {
        tapAngle = 0;
        animationIncomplete = false;
    }

    setTransform(QTransform()
                     .translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F)
                     .rotate(tapAngle)
                     .translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F));
    setHovered(false);
    update();

    return animationIncomplete;
}

QVariant CardItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == ItemSelectedHasChanged) && owner != nullptr) {
        bool selected = value.toBool();

        if (selected) {
            owner->getGame()->setActiveCard(this);
        }

        emit selectionChanged(this, selected);
    }

    return AbstractCardItem::itemChange(change, value);
}