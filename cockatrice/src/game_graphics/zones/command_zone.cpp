#include "command_zone.h"

#include "../../client/settings/cache_settings.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../../interface/theme_manager.h"
#include "../board/abstract_counter.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../board/commander_tax_counter.h"
#include "../z_values.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/utility/counter_ids.h>

CommandZone::CommandZone(CommandZoneLogic *_logic, int _zoneHeight, QGraphicsItem *parent)
    : SelectZone(_logic, parent), zoneHeight(_zoneHeight)
{
    connect(themeManager, &ThemeManager::themeChanged, this, &CommandZone::updateBg);
    updateBg();
    setCacheMode(DeviceCoordinateCache);
    setupClipContainer(ZValues::CARD_BASE);
}

CommandZone::~CommandZone()
{
    for (AbstractCounter *ctr : taxCounters) {
        disconnect(ctr, &QObject::destroyed, this, nullptr);
    }
}

void CommandZone::updateBg()
{
    update();
}

QRectF CommandZone::boundingRect() const
{
    return {0, 0, ZoneSizes::COMMAND_ZONE_WIDTH, currentHeight()};
}

qreal CommandZone::currentHeight() const
{
    return minimized ? qMax(zoneHeight * MINIMIZED_HEIGHT_RATIO, static_cast<double>(minimumHeight)) : zoneHeight;
}

void CommandZone::setMinimumHeight(int height)
{
    if (minimumHeight == height) {
        return;
    }
    // The floor only affects height while minimized; skip relayout if the displayed
    // height is unchanged (e.g. a tax counter toggled while expanded).
    const qreal oldEffectiveHeight = currentHeight();
    minimumHeight = height;
    if (qFuzzyCompare(currentHeight(), oldEffectiveHeight)) {
        return;
    }

    prepareGeometryChange();
    updateClipRect();
    reorganizeCards();
    update();
    // effectiveHeightChanged repositions neighbouring zones; that path re-enters with the
    // same floor and early-returns above, so it converges.
    emit effectiveHeightChanged();
}

bool CommandZone::isMinimized() const
{
    return minimized;
}

void CommandZone::toggleMinimized()
{
    minimized = !minimized;

    prepareGeometryChange();
    updateClipRect();
    reorganizeCards();
    update();

    emit minimizedChanged(minimized);
}

void CommandZone::paint(QPainter *painter,
                        [[maybe_unused]] const QStyleOptionGraphicsItem *option,
                        [[maybe_unused]] QWidget *widget)
{
    QBrush brush = themeManager->getExtraBgBrush(ThemeManager::Command, getLogic()->getPlayer()->getZoneId());

    QPointF scenePos = mapToScene(QPointF(0, 0));
    painter->setBrushOrigin(-scenePos);

    painter->fillRect(boundingRect(), brush);
}

void CommandZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                  CardZoneLogic *startZone,
                                  const QPoint &dropPoint)
{
    if (startZone == nullptr || startZone->getPlayer() == nullptr || dragItems.isEmpty()) {
        return;
    }

    int index = calcDropIndexFromY(dropPoint.y(), MIN_CARD_VISIBLE);

    // Same-zone no-op: don't move a card onto itself
    const auto &cards = getLogic()->getCards();
    if (!cards.isEmpty() && index < cards.size() && startZone == getLogic() &&
        cards.at(index)->getId() == dragItems.at(0)->getId()) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(getLogic()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_target_zone(getLogic()->getName().toStdString());
    cmd.set_x(index);
    cmd.set_y(0);

    for (const CardDragItem *item : dragItems) {
        if (item) {
            auto *cardToMove = cmd.mutable_cards_to_move()->add_card();
            cardToMove->set_card_id(item->getId());
            if (item->isForceFaceDown()) {
                cardToMove->set_face_down(true);
            }
        }
    }

    getLogic()->getPlayer()->getPlayerActions()->sendGameCommand(cmd);
}

void CommandZone::reorganizeCards()
{
    restoreStaleEscapedCards();
    updateClipRect();

    const auto &cards = getLogic()->getCards();
    if (cards.isEmpty()) {
        update();
        return;
    }

    auto params = buildStackParams(MIN_CARD_VISIBLE);
    params.allowBottomOverflow = true;
    layoutCardsVertically(params);
    update();
}

void CommandZone::registerTaxCounter(AbstractCounter *counter)
{
    if (!counter || taxCounters.contains(counter)) {
        return;
    }
    taxCounters.append(counter);
    connect(counter, &QObject::destroyed, this, [this, counter]() { taxCounters.removeOne(counter); });
}

void CommandZone::unregisterTaxCounter(AbstractCounter *counter)
{
    if (!counter) {
        return;
    }
    disconnect(counter, &QObject::destroyed, this, nullptr);
    taxCounters.removeOne(counter);
}

void CommandZone::rearrangeTaxCounters()
{
    int activeTaxCounterCount = 0;

    for (AbstractCounter *ctr : taxCounters) {
        qreal y = TaxCounterSizes::TAX_COUNTER_MARGIN +
                  activeTaxCounterCount * (TaxCounterSizes::TAX_COUNTER_SIZE + TaxCounterSizes::TAX_COUNTER_MARGIN);
        ctr->setPos(TaxCounterSizes::TAX_COUNTER_MARGIN, y);
        ctr->setZValue(ZValues::TAX_COUNTERS);
        // Visibility is owned solely by AbstractCounter::setActive() (the counter's own flag),
        // which Qt AND-s with this CommandZone's visibility via child-visibility propagation
        // (tax counters are graphics children of the zone). This function only handles layout,
        // so it stacks and measures by isActive() alone.
        if (ctr->isActive()) {
            ++activeTaxCounterCount;
        }
    }

    int minHeight = activeTaxCounterCount * (TaxCounterSizes::TAX_COUNTER_SIZE + TaxCounterSizes::TAX_COUNTER_MARGIN) +
                    TaxCounterSizes::TAX_COUNTER_MARGIN;
    setMinimumHeight(minHeight);
}

void CommandZone::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        toggleMinimized();
        event->accept();
    } else {
        SelectZone::mouseDoubleClickEvent(event);
    }
}
