#include "player_graphics_item.h"

// @see tests/command_zone/counter_visibility_test.cpp - test harness duplicates visibility logic
// @see tests/command_zone/command_zone_integration_test.cpp - test harness duplicates counter positioning

#include "../../interface/widgets/tabs/tab_game.h"
#include "../board/abstract_card_item.h"
#include "../board/commander_tax_counter.h"
#include "../hand_counter.h"
#include "../z_values.h"
#include "../zones/command_zone.h"
#include "../zones/hand_zone.h"
#include "../zones/pile_zone.h"
#include "../zones/stack_zone.h"
#include "../zones/table_zone.h"

#include <libcockatrice/common/counter_ids.h>

PlayerGraphicsItem::PlayerGraphicsItem(Player *_player) : player(_player)
{
    connect(&SettingsCache::instance(), &SettingsCache::horizontalHandChanged, this,
            &PlayerGraphicsItem::rearrangeZones);
    connect(&SettingsCache::instance(), &SettingsCache::handJustificationChanged, this,
            &PlayerGraphicsItem::rearrangeZones);
    connect(player, &Player::rearrangeCounters, this, &PlayerGraphicsItem::rearrangeCounters);

    playerArea = new PlayerArea(this);

    playerTarget = new PlayerTarget(player, playerArea);
    qreal avatarMargin = (counterAreaWidth + CARD_HEIGHT + 15 - playerTarget->boundingRect().width()) / 2.0;
    playerTarget->setPos(QPointF(avatarMargin, avatarMargin));

    initializeZones();

    connect(tableZoneGraphicsItem, &TableZone::sizeChanged, this, &PlayerGraphicsItem::updateBoundingRect);

    updateBoundingRect();

    rearrangeZones();
    retranslateUi();
}

void PlayerGraphicsItem::retranslateUi()
{
    player->getPlayerMenu()->retranslateUi();

    QMapIterator<QString, CardZoneLogic *> zoneIterator(player->getZones());
    while (zoneIterator.hasNext()) {
        emit zoneIterator.next().value()->retranslateUi();
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->retranslateUi();
    }
}

void PlayerGraphicsItem::onPlayerActiveChanged(bool _active)
{
    tableZoneGraphicsItem->setActive(_active);
}

void PlayerGraphicsItem::initializeZones()
{
    deckZoneGraphicsItem = new PileZone(player->getDeckZone(), this);
    auto base = QPointF(counterAreaWidth + (CARD_HEIGHT - CARD_WIDTH + 15) / 2.0,
                        10 + playerTarget->boundingRect().height() + 5 - (CARD_HEIGHT - CARD_WIDTH) / 2.0);
    deckZoneGraphicsItem->setPos(base);

    qreal h = deckZoneGraphicsItem->boundingRect().width() + 5;

    sideboardGraphicsItem = new PileZone(player->getSideboardZone(), this);
    player->getSideboardZone()->setGraphicsVisibility(false);

    auto *handCounter = new HandCounter(playerArea);
    handCounter->setPos(base + QPointF(0, h + 10));
    qreal h2 = handCounter->boundingRect().height();

    graveyardZoneGraphicsItem = new PileZone(player->getGraveZone(), this);
    graveyardZoneGraphicsItem->setPos(base + QPointF(0, h + h2 + 10));

    rfgZoneGraphicsItem = new PileZone(player->getRfgZone(), this);
    rfgZoneGraphicsItem->setPos(base + QPointF(0, 2 * h + h2 + 10));

    tableZoneGraphicsItem = new TableZone(player->getTableZone(), this);
    connect(tableZoneGraphicsItem, &TableZone::sizeChanged, this, &PlayerGraphicsItem::updateBoundingRect);

    stackZoneGraphicsItem =
        new StackZone(player->getStackZone(), static_cast<int>(tableZoneGraphicsItem->boundingRect().height()), this);

    handZoneGraphicsItem =
        new HandZone(player->getHandZone(), static_cast<int>(tableZoneGraphicsItem->boundingRect().height()), this);

    // Command zones - mixed hierarchy:
    // - Partner is Qt child of Command (tight coupling)
    // - Companion and Background are Qt siblings (direct children of PlayerGraphicsItem)
    //   with logical parent-child relationships for hierarchy traversal
    //
    // OWNERSHIP INVARIANT: All command zones must remain Qt children of either
    // CommandZone (for Partner) or PlayerGraphicsItem (for Primary, Companion, Background)
    // for the duration of their lifetime. The logical parent-child relationships
    // (logicalParentZone, logicalChildZones) are raw pointers that rely on this
    // ownership guarantee. Do not reparent zones or change destruction order.

    // Primary command zone
    commandZoneGraphicsItem =
        new CommandZone(player->getCommandZone(), ZoneSizes::COMMAND_ZONE_HEIGHT, CommandZoneType::Primary, this);
    commandZoneGraphicsItem->setZValue(ZValues::COMMAND_ZONE);
    commandZoneGraphicsItem->setVisible(false);
    connect(commandZoneGraphicsItem, &CommandZone::minimizedChanged, this, &PlayerGraphicsItem::rearrangeZones);

    // Partner zone is Qt child of Command (nested - preserves tight Command-Partner relationship)
    partnerZoneGraphicsItem = new CommandZone(player->getPartnerZone(), ZoneSizes::COMMAND_ZONE_HEIGHT,
                                              CommandZoneType::Partner, commandZoneGraphicsItem);
    partnerZoneGraphicsItem->setZValue(ZValues::PARTNER_ZONE);
    connect(partnerZoneGraphicsItem, &CommandZone::expandedChanged, this, &PlayerGraphicsItem::onZoneExpanded);
    connect(partnerZoneGraphicsItem, &CommandZone::minimizedChanged, this, &PlayerGraphicsItem::rearrangeZones);

    // Companion zone is SIBLING - direct child of PlayerGraphicsItem (fixes spacing bugs)
    companionZoneGraphicsItem =
        new CommandZone(player->getCompanionZone(), ZoneSizes::COMMAND_ZONE_HEIGHT, CommandZoneType::Companion, this);
    companionZoneGraphicsItem->setZValue(ZValues::PARTNER_ZONE + 1);
    companionZoneGraphicsItem->setVisible(false);
    connect(companionZoneGraphicsItem, &CommandZone::expandedChanged, this, &PlayerGraphicsItem::onZoneExpanded);
    connect(companionZoneGraphicsItem, &CommandZone::minimizedChanged, this, &PlayerGraphicsItem::rearrangeZones);

    // Background zone is SIBLING - direct child of PlayerGraphicsItem (fixes spacing bugs)
    backgroundZoneGraphicsItem =
        new CommandZone(player->getBackgroundZone(), ZoneSizes::COMMAND_ZONE_HEIGHT, CommandZoneType::Background, this);
    backgroundZoneGraphicsItem->setZValue(ZValues::PARTNER_ZONE + 2);
    backgroundZoneGraphicsItem->setVisible(false);
    connect(backgroundZoneGraphicsItem, &CommandZone::expandedChanged, this, &PlayerGraphicsItem::onZoneExpanded);
    connect(backgroundZoneGraphicsItem, &CommandZone::minimizedChanged, this, &PlayerGraphicsItem::rearrangeZones);

    // Set up logical parent-child relationships for sibling zones
    // (used by hierarchy traversal methods like tryMinimizeAbove, shouldPreventCollapse)
    companionZoneGraphicsItem->setLogicalParentZone(partnerZoneGraphicsItem);
    backgroundZoneGraphicsItem->setLogicalParentZone(companionZoneGraphicsItem);
    partnerZoneGraphicsItem->addLogicalChildZone(companionZoneGraphicsItem);
    companionZoneGraphicsItem->addLogicalChildZone(backgroundZoneGraphicsItem);

    connect(handZoneGraphicsItem->getLogic(), &HandZoneLogic::cardCountChanged, handCounter,
            &HandCounter::updateNumber);
    connect(handCounter, &HandCounter::showContextMenu, handZoneGraphicsItem, &HandZone::showContextMenu);
}

QRectF PlayerGraphicsItem::boundingRect() const
{
    return bRect;
}

qreal PlayerGraphicsItem::getMinimumWidth() const
{
    qreal result = tableZoneGraphicsItem->getMinimumWidth() + CARD_HEIGHT + 15 + counterAreaWidth +
                   stackZoneGraphicsItem->boundingRect().width();
    if (!SettingsCache::instance().getHorizontalHand()) {
        result += handZoneGraphicsItem->boundingRect().width();
    }
    return result;
}

void PlayerGraphicsItem::paint(QPainter * /*painter*/,
                               const QStyleOptionGraphicsItem * /*option*/,
                               QWidget * /*widget*/)
{
}

void PlayerGraphicsItem::processSceneSizeChange(int newPlayerWidth)
{
    qreal tableWidth =
        newPlayerWidth - CARD_HEIGHT - 15 - counterAreaWidth - stackZoneGraphicsItem->boundingRect().width();
    if (!SettingsCache::instance().getHorizontalHand()) {
        tableWidth -= handZoneGraphicsItem->boundingRect().width();
    }

    tableZoneGraphicsItem->setWidth(tableWidth);
    handZoneGraphicsItem->setWidth(tableWidth + stackZoneGraphicsItem->boundingRect().width());
}

void PlayerGraphicsItem::setMirrored(bool _mirrored)
{
    if (mirrored != _mirrored) {
        mirrored = _mirrored;
        rearrangeZones();
    }
}

void PlayerGraphicsItem::rearrangeCounters()
{
    qreal marginTop = 80;
    const qreal padding = 5;
    qreal ySize = boundingRect().y() + marginTop;

    bool commandZonesVisible = commandZoneGraphicsItem->isVisible();

    for (const auto &counter : player->getCounters()) {
        AbstractCounter *ctr = counter;

        if (ctr->getId() == CounterIds::CommanderTax) {
            if (commandZonesVisible) {
                ctr->setPos(2, 2);
                ctr->setZValue(ZValues::TAX_COUNTERS);
                ctr->setVisible(true);
            } else {
                ctr->setVisible(false);
            }
            continue;
        }

        if (ctr->getId() == CounterIds::PartnerTax) {
            if (commandZonesVisible && partnerZoneGraphicsItem->isExpanded()) {
                ctr->setPos(2, 2);
                ctr->setZValue(ZValues::TAX_COUNTERS);
                ctr->setVisible(true);
            } else {
                ctr->setVisible(false);
            }
            continue;
        }

        if (!ctr->getShownInCounterArea()) {
            continue;
        }

        QRectF br = ctr->boundingRect();
        ctr->setPos((counterAreaWidth - br.width()) / 2, ySize);
        ySize += br.height() + padding;
    }
}

qreal PlayerGraphicsItem::positionCommandZones(const QPointF &base, bool commandZonesVisible)
{
    qreal runningY = 0;

    // Position command/partner zones only if command zone is visible
    if (commandZonesVisible) {
        commandZoneGraphicsItem->setPos(base);
        partnerZoneGraphicsItem->setPos(0, commandZoneGraphicsItem->currentHeight());
        runningY = commandZoneGraphicsItem->currentHeight() + partnerZoneGraphicsItem->currentHeight();
    }

    // Companion and background are Qt siblings - position based on their own visibility
    bool companionZoneEnabled = companionZoneGraphicsItem->isVisible();
    bool backgroundZoneEnabled = backgroundZoneGraphicsItem->isVisible();

    if (companionZoneEnabled) {
        companionZoneGraphicsItem->setPos(base.x(), base.y() + runningY);
        runningY += companionZoneGraphicsItem->currentHeight();
    }

    if (backgroundZoneEnabled) {
        backgroundZoneGraphicsItem->setPos(base.x(), base.y() + runningY);
        runningY += backgroundZoneGraphicsItem->currentHeight();
    }

    // Toggle visibility rules with availability guards:
    // A zone's toggle is ONLY visible if:
    // 1. The zone itself is enabled (Qt visible), AND
    // 2. Either the zone is open OR it's the "next available" to open
    //
    // Truth table for entry point (game start, nothing open):
    // | Cmd Enabled | Comp Enabled | Bg Enabled | Entry Point Toggle |
    // |-------------|--------------|------------|--------------------|
    // | Y           | Y            | Y          | Partner            |
    // | Y           | N            | Y          | Partner            |
    // | N           | Y            | Y          | Companion          |
    // | N           | Y            | N          | Companion          |
    // | N           | N            | Y          | Background         |
    if (commandZonesVisible || companionZoneEnabled || backgroundZoneEnabled) {
        bool commandZoneEnabled = commandZonesVisible;

        bool partnerCollapsed = partnerZoneGraphicsItem->isCollapsed();
        bool companionCollapsed = companionZoneGraphicsItem->isCollapsed();
        bool backgroundCollapsed = backgroundZoneGraphicsItem->isCollapsed();

        bool partnerIsOpen = !partnerCollapsed;
        bool companionIsOpen = !companionCollapsed;
        bool backgroundIsOpen = !backgroundCollapsed;
        bool noDeeperZonesOpen = companionCollapsed && backgroundCollapsed;

        // Partner toggle: visible when open OR at game start (no deeper zones open)
        partnerZoneGraphicsItem->setToggleButtonVisible(commandZoneEnabled && (partnerIsOpen || noDeeperZonesOpen));

        // Companion toggle: visible when open OR when it's next in cascade OR entry point
        companionZoneGraphicsItem->setToggleButtonVisible(
            companionZoneEnabled &&
            (companionIsOpen || (partnerIsOpen && backgroundCollapsed) || (!commandZoneEnabled && noDeeperZonesOpen)));

        // Background toggle: visible when open OR parent open OR skipping disabled companion
        backgroundZoneGraphicsItem->setToggleButtonVisible(
            backgroundZoneEnabled && (backgroundIsOpen || companionIsOpen || (partnerIsOpen && !companionZoneEnabled) ||
                                      (!commandZoneEnabled && !companionZoneEnabled)));
    }

    stackZoneGraphicsItem->setPos(base + QPointF(0, runningY));

    return runningY;
}

void PlayerGraphicsItem::rearrangeZones()
{
    auto base = QPointF(CARD_HEIGHT + counterAreaWidth + 15, 0);

    bool commandZoneEnabled = commandZoneGraphicsItem->isVisible();
    bool companionZoneEnabled = companionZoneGraphicsItem->isVisible();
    bool backgroundZoneEnabled = backgroundZoneGraphicsItem->isVisible();
    bool anyCommandFamilyZonesVisible = commandZoneEnabled || companionZoneEnabled || backgroundZoneEnabled;

    // Calculate available stack height, accounting for command zones if present
    qreal tableHeight = tableZoneGraphicsItem->boundingRect().height();
    qreal stackHeight = tableHeight;
    if (anyCommandFamilyZonesVisible) {
        qreal totalCommandHeight = totalCommandZoneHeight();

        // Safety-net auto-minimize for non-expansion triggers (window resize, etc.)
        // Most overflow is now handled at expansion time by onZoneExpanded()
        constexpr int MAX_MINIMIZE_ITERATIONS = 3;
        int iterations = 0;
        while (totalCommandHeight > tableHeight - ZoneSizes::MINIMUM_STACK_HEIGHT &&
               iterations < MAX_MINIMIZE_ITERATIONS) {
            bool minimized = false;

            // Try command zone hierarchy first (includes partner via logical children)
            if (commandZoneEnabled) {
                minimized = commandZoneGraphicsItem->tryAutoMinimize();
            }

            // Fall back to companion/background directly when command zone is disabled.
            // These zones use toggleMinimized() directly since they don't have the same
            // hierarchical auto-minimize traversal that command zone provides.
            if (!minimized && companionZoneEnabled && companionZoneGraphicsItem->isExpanded() &&
                !companionZoneGraphicsItem->isMinimized()) {
                companionZoneGraphicsItem->toggleMinimized();
                minimized = true;
            }

            if (!minimized && backgroundZoneEnabled && backgroundZoneGraphicsItem->isExpanded() &&
                !backgroundZoneGraphicsItem->isMinimized()) {
                backgroundZoneGraphicsItem->toggleMinimized();
                minimized = true;
            }

            if (!minimized)
                break;

            totalCommandHeight = totalCommandZoneHeight();
            ++iterations;
        }

        stackHeight = tableHeight - totalCommandHeight;
        if (stackHeight < ZoneSizes::MINIMUM_STACK_HEIGHT) {
            stackHeight = ZoneSizes::MINIMUM_STACK_HEIGHT;
        }
    }
    stackZoneGraphicsItem->setHeight(stackHeight);

    if (SettingsCache::instance().getHorizontalHand()) {
        if (mirrored) {
            if (player->getHandZone()->contentsKnown()) {
                player->getPlayerInfo()->setHandVisible(true);
                handZoneGraphicsItem->setPos(base);
                base += QPointF(0, handZoneGraphicsItem->boundingRect().height());
            } else {
                player->getPlayerInfo()->setHandVisible(false);
            }

            positionCommandZones(base, commandZoneEnabled);
            base += QPointF(stackZoneGraphicsItem->boundingRect().width(), 0);

            tableZoneGraphicsItem->setPos(base);
        } else {
            positionCommandZones(base, commandZoneEnabled);

            tableZoneGraphicsItem->setPos(base.x() + stackZoneGraphicsItem->boundingRect().width(), 0);
            base += QPointF(0, tableZoneGraphicsItem->boundingRect().height());

            if (player->getHandZone()->contentsKnown()) {
                player->getPlayerInfo()->setHandVisible(true);
                handZoneGraphicsItem->setPos(base);
            } else {
                player->getPlayerInfo()->setHandVisible(false);
            }
        }
        handZoneGraphicsItem->setWidth(tableZoneGraphicsItem->getWidth() +
                                       stackZoneGraphicsItem->boundingRect().width());
    } else {
        player->getPlayerInfo()->setHandVisible(true);

        handZoneGraphicsItem->setPos(base);
        base += QPointF(handZoneGraphicsItem->boundingRect().width(), 0);

        positionCommandZones(base, commandZoneEnabled);
        base += QPointF(stackZoneGraphicsItem->boundingRect().width(), 0);

        tableZoneGraphicsItem->setPos(base);
    }
    handZoneGraphicsItem->setVisible(player->getPlayerInfo()->getHandVisible());
    handZoneGraphicsItem->updateOrientation();
    tableZoneGraphicsItem->reorganizeCards();
    updateBoundingRect();
    rearrangeCounters();
}

void PlayerGraphicsItem::updateBoundingRect()
{
    prepareGeometryChange();
    qreal width = CARD_HEIGHT + 15 + counterAreaWidth + stackZoneGraphicsItem->boundingRect().width();
    if (SettingsCache::instance().getHorizontalHand()) {
        qreal handHeight =
            player->getPlayerInfo()->getHandVisible() ? handZoneGraphicsItem->boundingRect().height() : 0;
        bRect = QRectF(0, 0, width + tableZoneGraphicsItem->boundingRect().width(),
                       tableZoneGraphicsItem->boundingRect().height() + handHeight);
    } else {
        bRect = QRectF(
            0, 0, width + handZoneGraphicsItem->boundingRect().width() + tableZoneGraphicsItem->boundingRect().width(),
            tableZoneGraphicsItem->boundingRect().height());
    }
    playerArea->setSize(CARD_HEIGHT + counterAreaWidth + 15, bRect.height());

    emit sizeChanged();
}

qreal PlayerGraphicsItem::totalCommandZoneHeight() const
{
    qreal total = 0;

    // Command zone and partner use Qt parent-child - checking command visibility
    // implicitly covers partner (Qt hides children when parent is hidden)
    if (commandZoneGraphicsItem->isVisible()) {
        total += commandZoneGraphicsItem->currentHeight();
        if (partnerZoneGraphicsItem->isExpanded()) {
            total += partnerZoneGraphicsItem->currentHeight();
        }
    }

    // Sibling zones have independent visibility - check each one
    if (companionZoneGraphicsItem->isVisible()) {
        total += companionZoneGraphicsItem->currentHeight();
    }

    if (backgroundZoneGraphicsItem->isVisible()) {
        total += backgroundZoneGraphicsItem->currentHeight();
    }

    return total;
}

void PlayerGraphicsItem::onZoneExpanded(bool expanded)
{
    if (!expanded) {
        rearrangeZones();
        return;
    }

    auto *expandedZone = qobject_cast<CommandZone *>(sender());
    if (expandedZone == nullptr) {
        rearrangeZones();
        return;
    }

    qreal tableHeight = tableZoneGraphicsItem->boundingRect().height();
    qreal totalHeight = totalCommandZoneHeight();

    constexpr int MAX_MINIMIZE_ITERATIONS = 3;
    int iterations = 0;
    while (totalHeight > tableHeight - ZoneSizes::MINIMUM_STACK_HEIGHT && iterations < MAX_MINIMIZE_ITERATIONS) {
        if (!expandedZone->tryMinimizeAbove())
            break;

        totalHeight = totalCommandZoneHeight();
        ++iterations;
    }

    rearrangeZones();
}

void PlayerGraphicsItem::setCommandZonesVisible(bool visible)
{
    if (commandZoneGraphicsItem) {
        commandZoneGraphicsItem->setVisible(visible);
    }

    // Reset partner zone to collapsed state when hiding
    if (!visible && partnerZoneGraphicsItem) {
        partnerZoneGraphicsItem->setExpanded(false);
    }
    rearrangeZones();
}

void PlayerGraphicsItem::setCompanionZoneVisible(bool visible)
{
    if (companionZoneGraphicsItem) {
        companionZoneGraphicsItem->setVisible(visible);
        if (!visible) {
            companionZoneGraphicsItem->setExpanded(false);
        }
    }
    rearrangeZones();
}

void PlayerGraphicsItem::setBackgroundZoneVisible(bool visible)
{
    if (backgroundZoneGraphicsItem) {
        backgroundZoneGraphicsItem->setVisible(visible);
        if (!visible) {
            backgroundZoneGraphicsItem->setExpanded(false);
        }
    }
    rearrangeZones();
}
