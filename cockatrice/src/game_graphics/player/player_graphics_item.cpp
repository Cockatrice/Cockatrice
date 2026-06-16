#include "player_graphics_item.h"

#include "../../game/player/player_actions.h"
#include "../../interface/widgets/tabs/tab_game.h"
#include "../board/abstract_card_item.h"
#include "../board/commander_tax_counter.h"
#include "../board/counter_general.h"
#include "../hand_counter.h"
#include "../z_values.h"
#include "../zones/command_zone.h"
#include "../zones/hand_zone.h"
#include "../zones/pile_zone.h"
#include "../zones/stack_zone.h"
#include "../zones/table_zone.h"
#include "menu/player_menu.h"
#include "player_dialogs.h"

#include <QGraphicsView>
#include <libcockatrice/utility/counter_ids.h>

PlayerGraphicsItem::PlayerGraphicsItem(PlayerLogic *_player) : player(_player)
{
    connect(&SettingsCache::instance(), &SettingsCache::horizontalHandChanged, this,
            &PlayerGraphicsItem::rearrangeZones);
    connect(&SettingsCache::instance(), &SettingsCache::handJustificationChanged, this,
            &PlayerGraphicsItem::rearrangeZones);
    connect(player, &PlayerLogic::rearrangeCounters, this, &PlayerGraphicsItem::rearrangeCounters);
    connect(player, &PlayerLogic::activeChanged, this, &PlayerGraphicsItem::onPlayerActiveChanged);
    connect(player, &PlayerLogic::concededChanged, this, [this](int, bool c) { setVisible(!c); });
    connect(player, &PlayerLogic::zoneIdChanged, this, [this](int id) { playerArea->setPlayerZoneId(id); });

    connect(player, &PlayerLogic::counterAdded, this, &PlayerGraphicsItem::onCounterAdded);
    connect(player, &PlayerLogic::counterRemoved, this, &PlayerGraphicsItem::onCounterRemoved);

    playerMenu = new PlayerMenu(this);

    connect(playerMenu, &PlayerMenu::shortcutsActivated, this, [this]() {
        for (auto *ctr : counterWidgets) {
            ctr->setShortcutsActive();
        }
    });
    connect(playerMenu, &PlayerMenu::shortcutsDeactivated, this, [this]() {
        for (auto *ctr : counterWidgets) {
            ctr->setShortcutsInactive();
        }
    });
    connect(playerMenu, &PlayerMenu::retranslateRequested, this, [this]() {
        for (auto *ctr : counterWidgets) {
            ctr->retranslateUi();
        }
    });

    playerDialogs = new PlayerDialogs(this, player->getPlayerActions());

    connect(playerDialogs, &PlayerDialogs::requestDialogSemaphore, player, &PlayerLogic::setDialogSemaphore);

    playerArea = new PlayerArea(this);

    playerTarget = new PlayerTarget(player, playerArea);
    qreal avatarMargin =
        (counterAreaWidth + CardDimensions::HEIGHT_F + 15 - playerTarget->boundingRect().width()) / 2.0;
    playerTarget->setPos(QPointF(avatarMargin, avatarMargin));

    initializeZones();

    connect(player, &PlayerLogic::addViewCustomZoneActionToCustomZoneMenu, this,
            &PlayerGraphicsItem::onCustomZoneAdded);
    connect(player, &PlayerLogic::commandZoneSupportChanged, this, &PlayerGraphicsItem::setCommandZoneVisible);

    playerMenu->setMenusForGraphicItems();

    connect(tableZoneGraphicsItem, &TableZone::sizeChanged, this, &PlayerGraphicsItem::updateBoundingRect);

    updateBoundingRect();

    rearrangeZones();
    retranslateUi();
}

void PlayerGraphicsItem::retranslateUi()
{
    playerMenu->retranslateUi();

    QMapIterator<QString, CardZoneLogic *> zoneIterator(player->getZones());
    while (zoneIterator.hasNext()) {
        emit zoneIterator.next().value()->retranslateUi();
    }
}

void PlayerGraphicsItem::onPlayerActiveChanged(bool _active)
{
    tableZoneGraphicsItem->setActive(_active);
}

void PlayerGraphicsItem::initializeZones()
{
    deckZoneGraphicsItem = new PileZone(player->getDeckZone(), this);
    auto base = QPointF(counterAreaWidth + (CardDimensions::HEIGHT_F - CardDimensions::WIDTH_F + 15) / 2.0,
                        10 + playerTarget->boundingRect().height() + 5 -
                            (CardDimensions::HEIGHT_F - CardDimensions::WIDTH_F) / 2.0);
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

    tableZoneGraphicsItem = new TableZone(player->getTableZone(), mirrored, this);
    connect(tableZoneGraphicsItem, &TableZone::sizeChanged, this, &PlayerGraphicsItem::updateBoundingRect);
    connect(this, &PlayerGraphicsItem::mirroredChanged, tableZoneGraphicsItem, &TableZone::setMirrored);

    stackZoneGraphicsItem =
        new StackZone(player->getStackZone(), static_cast<int>(tableZoneGraphicsItem->boundingRect().height()), this);

    handZoneGraphicsItem =
        new HandZone(player->getHandZone(), static_cast<int>(tableZoneGraphicsItem->boundingRect().height()), this);
    connect(player->getPlayerActions(), &PlayerActions::requestSortHand, handZoneGraphicsItem, &HandZone::sortHand);

    // Command zone
    commandZoneGraphicsItem = new CommandZone(player->getCommandZone(), ZoneSizes::COMMAND_ZONE_HEIGHT, this);
    commandZoneGraphicsItem->setZValue(ZValues::COMMAND_ZONE);
    commandZoneGraphicsItem->setVisible(false);
    connect(commandZoneGraphicsItem, &CommandZone::minimizedChanged, this, &PlayerGraphicsItem::rearrangeZones);

    connect(handZoneGraphicsItem->getLogic(), &HandZoneLogic::cardCountChanged, handCounter,
            &HandCounter::updateNumber);
    connect(handCounter, &HandCounter::showContextMenu, handZoneGraphicsItem, &HandZone::showContextMenu);

    zoneGraphicsItems.insert(player->getDeckZone()->getName(), deckZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getGraveZone()->getName(), graveyardZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getRfgZone()->getName(), rfgZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getSideboardZone()->getName(), sideboardGraphicsItem);
    zoneGraphicsItems.insert(player->getTableZone()->getName(), tableZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getStackZone()->getName(), stackZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getHandZone()->getName(), handZoneGraphicsItem);
    zoneGraphicsItems.insert(player->getCommandZone()->getName(), commandZoneGraphicsItem);
}

void PlayerGraphicsItem::onCustomZoneAdded(QString customZoneName)
{
    zoneGraphicsItems.insert(customZoneName, nullptr); // Custom zone view goes here, if we ever implement it.
}

QRectF PlayerGraphicsItem::boundingRect() const
{
    return bRect;
}

qreal PlayerGraphicsItem::getMinimumWidth() const
{
    qreal result = tableZoneGraphicsItem->getMinimumWidth() + CardDimensions::HEIGHT_F + 15 + counterAreaWidth +
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
    // Extend table (and hand, if horizontal) to accommodate the new player width.
    qreal tableWidth = newPlayerWidth - CardDimensions::HEIGHT_F - 15 - counterAreaWidth -
                       stackZoneGraphicsItem->boundingRect().width();
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
        emit mirroredChanged(mirrored);
        rearrangeZones();
    }
}

void PlayerGraphicsItem::onCounterAdded(CounterState *state)
{
    AbstractCounter *widget;
    if (state->getName() == "life") {
        widget = playerTarget->addCounter(state);
    } else if (CounterNames::isTaxCounter(state->getName())) {
        if (!commandZoneGraphicsItem) {
            qWarning() << "Cannot create tax counter" << state->getName() << "- command zone not available";
            return;
        }
        // Qt parent (commandZoneGraphicsItem) owns widget; counterWidgets map holds reference
        // for lookup; CommandZone::registerTaxCounter connects QObject::destroyed for cleanup
        widget = new CommanderTaxCounter(state, player, commandZoneGraphicsItem);
        widget->setActive(state->isActive());
        commandZoneGraphicsItem->registerTaxCounter(widget);
    } else {
        widget = new GeneralCounter(state, player, true, this);
    }
    counterWidgets.insert(state->getId(), widget);

    if (playerMenu->getCountersMenu() && widget->getMenu()) {
        playerMenu->getCountersMenu()->addMenu(widget->getMenu());
    }

    if (playerMenu->getShortcutsActive()) {
        widget->setShortcutsActive();
    }

    rearrangeCounters();
}

void PlayerGraphicsItem::onCounterRemoved(int counterId)
{
    auto *widget = counterWidgets.take(counterId);
    if (!widget) {
        return;
    }
    if (playerMenu->getCountersMenu() && widget->getMenu()) {
        playerMenu->getCountersMenu()->removeAction(widget->getMenu()->menuAction());
    }
    widget->delCounter();
    rearrangeCounters();
}

void PlayerGraphicsItem::rearrangeCounters()
{
    if (commandZoneGraphicsItem) {
        commandZoneGraphicsItem->rearrangeTaxCounters();
    }

    qreal ySize = boundingRect().y() + 80;
    constexpr qreal padding = 5;
    for (auto *ctr : counterWidgets.values()) {
        if (CounterNames::isTaxCounter(ctr->getName())) {
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

AbstractCounter *PlayerGraphicsItem::getTaxCounterIfActive(int counterId) const
{
    AbstractCounter *counter = getCounterWidget(counterId);
    return (counter && counter->isActive()) ? counter : nullptr;
}

void PlayerGraphicsItem::rearrangeZones()
{
    auto base = QPointF(CardDimensions::HEIGHT_F + counterAreaWidth + 15, 0);

    // Calculate stack height, accounting for command zone if visible
    bool commandZoneVisible = commandZoneGraphicsItem && commandZoneGraphicsItem->isVisible();
    qreal tableHeight = tableZoneGraphicsItem->boundingRect().height();
    qreal stackHeight = tableHeight;
    if (commandZoneVisible) {
        stackHeight = tableHeight - totalCommandZoneHeight();
        if (stackHeight < CommandZone::MINIMUM_STACKING_HEIGHT) {
            stackHeight = CommandZone::MINIMUM_STACKING_HEIGHT;
        }
    }
    stackZoneGraphicsItem->setHeight(stackHeight);

    if (SettingsCache::instance().getHorizontalHand()) {
        if (mirrored) {
            if (player->getHandZone()->contentsKnown()) {
                handVisible = true;
                handZoneGraphicsItem->setPos(base);
                base += QPointF(0, handZoneGraphicsItem->boundingRect().height());
            } else {
                handVisible = false;
            }

            positionCommandAndStackZones(base);
            base += QPointF(stackZoneGraphicsItem->boundingRect().width(), 0);

            tableZoneGraphicsItem->setPos(base);
        } else {
            positionCommandAndStackZones(base);

            tableZoneGraphicsItem->setPos(base.x() + stackZoneGraphicsItem->boundingRect().width(), 0);
            base += QPointF(0, tableZoneGraphicsItem->boundingRect().height());

            if (player->getHandZone()->contentsKnown()) {
                handVisible = true;
                handZoneGraphicsItem->setPos(base);
            } else {
                handVisible = false;
            }
        }
        handZoneGraphicsItem->setWidth(tableZoneGraphicsItem->getWidth() +
                                       stackZoneGraphicsItem->boundingRect().width());
    } else {
        handVisible = true;

        handZoneGraphicsItem->setPos(base);
        base += QPointF(handZoneGraphicsItem->boundingRect().width(), 0);

        positionCommandAndStackZones(base);
        base += QPointF(stackZoneGraphicsItem->boundingRect().width(), 0);

        tableZoneGraphicsItem->setPos(base);
    }
    handZoneGraphicsItem->setVisible(handVisible);
    handZoneGraphicsItem->updateOrientation();
    tableZoneGraphicsItem->reorganizeCards();
    updateBoundingRect();
    rearrangeCounters();
}

void PlayerGraphicsItem::updateBoundingRect()
{
    prepareGeometryChange();
    qreal width = CardDimensions::HEIGHT_F + 15 + counterAreaWidth + stackZoneGraphicsItem->boundingRect().width();
    if (SettingsCache::instance().getHorizontalHand()) {
        qreal handHeight = handVisible ? handZoneGraphicsItem->boundingRect().height() : 0;
        bRect = QRectF(0, 0, width + tableZoneGraphicsItem->boundingRect().width(),
                       tableZoneGraphicsItem->boundingRect().height() + handHeight);
    } else {
        bRect = QRectF(
            0, 0, width + handZoneGraphicsItem->boundingRect().width() + tableZoneGraphicsItem->boundingRect().width(),
            tableZoneGraphicsItem->boundingRect().height());
    }
    playerArea->setSize(CardDimensions::HEIGHT_F + counterAreaWidth + 15, bRect.height());

    emit sizeChanged();
}

qreal PlayerGraphicsItem::totalCommandZoneHeight() const
{
    if (commandZoneGraphicsItem && commandZoneGraphicsItem->isVisible()) {
        return commandZoneGraphicsItem->currentHeight();
    }
    return 0;
}

void PlayerGraphicsItem::positionCommandAndStackZones(const QPointF &base)
{
    bool commandZoneVisible = commandZoneGraphicsItem && commandZoneGraphicsItem->isVisible();
    if (commandZoneVisible) {
        commandZoneGraphicsItem->setPos(base);
    }
    stackZoneGraphicsItem->setPos(base.x(), base.y() + (commandZoneVisible ? totalCommandZoneHeight() : 0));
}

void PlayerGraphicsItem::setCommandZoneVisible(bool visible)
{
    if (commandZoneGraphicsItem) {
        commandZoneGraphicsItem->setVisible(visible);
    }
    rearrangeZones();
}
