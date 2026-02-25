#include "command_zone.h"

// @see CommandZoneState for the extracted visibility/height state machine

#include "../../client/settings/cache_settings.h"
#include "../../interface/theme_manager.h"
#include "../board/abstract_counter.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../player/player.h"
#include "../player/player_actions.h"
#include "../z_values.h"
#include "select_zone.h"
#include "zone_toggle_button.h"

#include <QDebug>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <libcockatrice/common/counter_ids.h>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>

CommandZone::CommandZone(CommandZoneLogic *_logic, int _zoneHeight, CommandZoneType _zoneType, QGraphicsItem *parent)
    : SelectZone(_logic, parent), m_state(_zoneHeight, _zoneType), toggleButton(nullptr), cardClipContainer(nullptr)
{
    connect(themeManager, &ThemeManager::themeChanged, this, &CommandZone::updateBg);
    updateBg();
    setCacheMode(DeviceCoordinateCache);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);

    cardClipContainer = new QGraphicsRectItem(this);
    cardClipContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    cardClipContainer->setPen(Qt::NoPen);
    cardClipContainer->setBrush(Qt::NoBrush);
    cardClipContainer->setRect(boundingRect());
    cardClipContainer->setZValue(ZValues::CARD_BASE);

    disconnect(getLogic(), &CardZoneLogic::cardAdded, this, &CardZone::onCardAdded);
    connect(getLogic(), &CardZoneLogic::cardAdded, this, [this](CardItem *card) {
        if (cardClipContainer && card) {
            card->setParentItem(cardClipContainer);
            card->update();
        }
    });

    if (!isPrimary()) {
        toggleButton = new ZoneToggleButton(getZoneType(), this);
        connect(toggleButton, &ZoneToggleButton::clicked, this, &CommandZone::toggleExpanded);
        repositionToggleButton();
    }
}

CommandZone::~CommandZone()
{
    disconnect(getLogic(), &CardZoneLogic::cardAdded, this, nullptr);
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
    return m_state.currentHeight();
}

bool CommandZone::isPrimary() const
{
    return m_state.isPrimary();
}

bool CommandZone::isMinimized() const
{
    return m_state.isMinimized();
}

bool CommandZone::isExpanded() const
{
    return m_state.isExpanded();
}

bool CommandZone::isCollapsed() const
{
    return m_state.isCollapsed();
}

void CommandZone::toggleExpanded()
{
    applyStateChange(m_state.tryToggleExpanded(shouldPreventCollapse()));
}

void CommandZone::toggleMinimized()
{
    applyStateChange(m_state.tryToggleMinimized(), true);
}

void CommandZone::applyStateChange(const StateChangeResult &result, bool updateTogglePosition)
{
    if (!result.geometryChanged) {
        return;
    }

    prepareGeometryChange();
    if (cardClipContainer) {
        cardClipContainer->setRect(boundingRect());
    }
    if (toggleButton) {
        toggleButton->setExpanded(m_state.isExpanded());
    }
    reorganizeCards();
    if (updateTogglePosition) {
        repositionToggleButton();
    }
    update();

    if (result.shouldEmitMinimized) {
        emit minimizedChanged(m_state.isMinimized());
    }
    if (result.shouldEmitExpanded) {
        emit expandedChanged(m_state.isExpanded());
    }
}

void CommandZone::paint(QPainter *painter,
                        [[maybe_unused]] const QStyleOptionGraphicsItem *option,
                        [[maybe_unused]] QWidget *widget)
{
    if (isCollapsed()) {
        return;
    }

    QBrush brush = themeManager->getExtraBgBrush(ThemeManager::Command, getLogic()->getPlayer()->getZoneId());

    QPointF scenePos = mapToScene(QPointF(0, 0));
    painter->setBrushOrigin(-scenePos);

    painter->fillRect(boundingRect(), brush);
}

void CommandZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                  CardZoneLogic *startZone,
                                  const QPoint &dropPoint)
{
    if (dragItems.isEmpty()) {
        return;
    }

    if (startZone == nullptr || startZone->getPlayer() == nullptr) {
        qCWarning(CommandZoneLog) << "handleDropEvent: Invalid startZone or player";
        return;
    }

    // Primary and Partner zones hold exactly one card; Companion and Background support multiple
    if (!supportsMultipleCards(getZoneType()) && !getLogic()->getCards().isEmpty()) {
        qCDebug(CommandZoneLog) << "handleDropEvent: Single-card zone already occupied, ignoring drop";
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(getLogic()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_target_zone(getLogic()->getName().toStdString());

    // Calculate insertion index from drop position for multi-card zones
    int index = 0;
    const auto &cards = getLogic()->getCards();
    if (!cards.isEmpty()) {
        const auto cardCount = static_cast<int>(cards.size());
        const qreal cardHeight = cards.at(0)->boundingRect().height();

        index = qRound(divideCardSpaceInZone(dropPoint.y(), cardCount, boundingRect().height(), cardHeight, true));
        index = qBound(0, index, cardCount);
    }

    cmd.set_x(index);
    cmd.set_y(0);

    // Add all dragged cards to the move command
    for (const CardDragItem *item : dragItems) {
        if (item != nullptr) {
            cmd.mutable_cards_to_move()->add_card()->set_card_id(item->getId());
        }
    }

    getLogic()->getPlayer()->getPlayerActions()->sendGameCommand(cmd);
}

void CommandZone::reorganizeCards()
{
    const auto &cards = getLogic()->getCards();

    if (cardClipContainer) {
        cardClipContainer->setRect(boundingRect());
    }

    // Hide all cards when collapsed
    if (isCollapsed()) {
        for (CardItem *card : cards) {
            if (card != nullptr) {
                card->setVisible(false);
            }
        }
        update();
        return;
    }

    if (cards.isEmpty()) {
        update();
        return;
    }

    const auto cardCount = static_cast<int>(cards.size());
    const qreal totalWidth = boundingRect().width();
    const qreal totalHeight = boundingRect().height();
    const qreal cardWidth = cards.at(0)->boundingRect().width();
    const qreal cardHeight = cards.at(0)->boundingRect().height();

    // Zigzag horizontal positioning (StackZone pattern)
    // Single card stays centered for visual consistency with Primary/Partner zones
    const qreal xMargin = 5;
    const qreal x1 = xMargin;
    const qreal x2 = totalWidth - xMargin - cardWidth;

    for (int i = 0; i < cardCount; i++) {
        CardItem *card = cards.at(i);
        if (card == nullptr) {
            continue;
        }

        // Horizontal: center single card, zigzag for multiple
        qreal x = (cardCount == 1) ? (totalWidth - cardWidth) / 2.0 : ((i % 2) ? x2 : x1);

        // Vertical: use divideCardSpaceInZone for proper stacking with overlap compression.
        // divideCardSpaceInZone can return negative Y when totalHeight < cardHeight.
        // Clamp to 0 so card tops remain visible when clipped by cardClipContainer.
        const qreal y = qMax(0.0, divideCardSpaceInZone(i, cardCount, totalHeight, cardHeight));

        card->setVisible(true);
        card->setPos(x, y);
        card->setRealZValue(ZValues::CARD_BASE + i);
    }
    update();
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

void CommandZone::repositionToggleButton()
{
    if (toggleButton) {
        QRectF btnRect = toggleButton->boundingRect();
        qreal btnX = (boundingRect().width() - btnRect.width()) / 2.0;
        qreal btnY = 0;
        toggleButton->setPos(btnX, btnY);
    }
}

void CommandZone::setExpanded(bool _expanded)
{
    applyStateChange(m_state.trySetExpanded(_expanded, shouldPreventCollapse()));
}

bool CommandZone::shouldPreventCollapse() const
{
    if (isPrimary())
        return false;

    if (!getLogic()->getCards().isEmpty())
        return true;

    if (getZoneType() == CommandZoneType::Partner) {
        Player *player = getLogic()->getPlayer();
        if (player) {
            AbstractCounter *partnerTax = player->getCounters().value(CounterIds::PartnerTax, nullptr);
            if (partnerTax && partnerTax->getValue() > 0)
                return true;
        }
    }

    // Check Qt children (Partner has toggle button as child, and Partner is nested under Command)
    for (QGraphicsItem *child : childItems()) {
        if (auto *childZone = dynamic_cast<CommandZone *>(child)) {
            if (childZone->shouldPreventCollapse())
                return true;
        }
    }

    // Check logical children (for sibling zones: Companion is logical child of Partner,
    // Background is logical child of Companion)
    for (CommandZone *childZone : logicalChildZones) {
        if (childZone != nullptr && childZone->shouldPreventCollapse())
            return true;
    }

    return false;
}

bool CommandZone::tryAutoMinimize()
{
    // Check Qt children first (Partner is nested under Command)
    for (QGraphicsItem *child : childItems()) {
        if (auto *childZone = dynamic_cast<CommandZone *>(child)) {
            if (childZone->tryAutoMinimize())
                return true;
        }
    }

    // Check logical children (sibling zones: Companion under Partner, Background under Companion)
    for (CommandZone *childZone : logicalChildZones) {
        if (childZone != nullptr && childZone->tryAutoMinimize())
            return true;
    }

    if (isExpanded() && !isMinimized() && !isPrimary()) {
        toggleMinimized();
        return true;
    }

    return false;
}

CommandZone *CommandZone::getParentCommandZone() const
{
    QGraphicsItem *parent = parentItem();
    while (parent != nullptr) {
        if (auto *parentZone = dynamic_cast<CommandZone *>(parent)) {
            return parentZone;
        }
        parent = parent->parentItem();
    }
    return nullptr;
}

bool CommandZone::tryMinimizeAbove()
{
    // First check logical parent (for sibling zones: Companion, Background)
    CommandZone *parentZone = logicalParentZone;

    // Fall back to Qt parent chain for Partner zone (still nested under Command)
    if (parentZone == nullptr) {
        parentZone = getParentCommandZone();
    }

    if (parentZone != nullptr && parentZone->isExpanded() && !parentZone->isMinimized()) {
        parentZone->toggleMinimized();
        return true;
    }
    // If parent can't minimize, try grandparent (recursion bounded by zone depth)
    if (parentZone != nullptr) {
        return parentZone->tryMinimizeAbove();
    }
    return false;
}

void CommandZone::setToggleButtonVisible(bool visible)
{
    if (toggleButton) {
        toggleButton->setVisible(visible);
    }
}
