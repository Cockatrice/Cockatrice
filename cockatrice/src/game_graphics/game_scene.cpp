#include "game_scene.h"

#include "../client/settings/cache_settings.h"
#include "../interface/widgets/cards/card_hover_widget.h"
#include "animated_item.h"
#include "board/abstract_card_item.h"
#include "board/arrow_item.h"
#include "board/arrow_registry.h"
#include "board/card_item.h"
#include "phases_toolbar.h"
#include "player/menu/player_menu.h"
#include "player/player_graphics_item.h"
#include "player/player_target.h"
#include "z_values.h"
#include "zones/card_zone.h"
#include "zones/select_zone.h"
#include "zones/view_zone.h"
#include "zones/view_zone_widget.h"

#include <QBasicTimer>
#include <QCursor>
#include <QDebug>
#include <QEvent>
#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsView>
#include <QListIterator>
#include <QMenu>
#include <QMessageLogger>
#include <QMutableHashIterator>
#include <QObject>
#include <QPoint>
#include <QRectF>
#include <QScrollBar>
#include <QSet>
#include <QSharedPointer>
#include <QTimer>
#include <QtGlobal>
#include <algorithm>
#include <functional>
#include <libcockatrice/game/abstract_game.h>
#include <libcockatrice/game/board/arrow_data.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_event_handler.h>
#include <libcockatrice/game/player/player_info.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/zones/card_zone_logic.h>
#include <libcockatrice/game/zones/view_zone_logic.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/utility/zone_names.h>
#include <numeric>
#include <qnamespace.h>

class ArrowTarget;

/**
 * @brief Constructs the GameScene.
 * @param _phasesToolbar Toolbar widget for phases.
 * @param parent Optional parent QObject.
 *
 * Initializes the animation timer, adds the phases toolbar to the scene,
 * and connects to settings changes for multi-column layout.
 * Finally, calls rearrange() to layout players initially.
 */
GameScene::GameScene(PhasesToolbar *_phasesToolbar, QObject *parent)
    : QGraphicsScene(parent), phasesToolbar(_phasesToolbar), viewSize(QSize()), playerRotation(0)
{
    animationTimer = new QBasicTimer;
    addItem(phasesToolbar);
    connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::minPlayersForMultiColumnLayoutChanged, this,
            &GameScene::rearrange);

    // Card hover popup (cardHoverWidget is created lazily on first show)
    hoverActivateTimer = new QTimer(this);
    hoverActivateTimer->setSingleShot(true);
    hoverActivateTimer->setInterval(500);
    connect(hoverActivateTimer, &QTimer::timeout, this, &GameScene::onHoverActivateTimeout);

    hoverPanelTimer = new QTimer(this);
    hoverPanelTimer->setSingleShot(true);
    hoverPanelTimer->setInterval(200);
    connect(hoverPanelTimer, &QTimer::timeout, this, &GameScene::onHoverPanelTimeout);

    hoverGraceTimer = new QTimer(this);
    hoverGraceTimer->setSingleShot(true);
    hoverGraceTimer->setInterval(200);
    connect(hoverGraceTimer, &QTimer::timeout, this, &GameScene::onHoverGraceTimeout);

    rearrange();
}

GameScene::~GameScene()
{
    // Sever all destroyed->removeAnimatedItem connections before the members below
    // are destroyed: the base QGraphicsScene destructor destroys the remaining items,
    // and their destroyed() signals must not reach slots that reference members that
    // no longer exist. The connection handle overload is used because the string-based
    // disconnect(nullptr, nullptr, this, nullptr) is invalid (the sender must never be
    // nullptr) and would otherwise fail to sever these pointer-to-member connections.
    for (auto it = animationItemConnections.constBegin(); it != animationItemConnections.constEnd(); ++it) {
        QObject::disconnect(*it);
    }
    animationItemConnections.clear();

    delete animationTimer;
    animationTimer = nullptr;
    delete hoverActivateTimer;
    delete hoverPanelTimer;
    delete hoverGraceTimer;

    // Delete all ArrowItems before QGraphicsScene's base destructor runs.
    // QGraphicsScene::~QGraphicsScene() destroys items in arbitrary order.
    // If a PlayerTarget is destroyed before an ArrowItem pointing to it,
    // ArrowItem::onTargetDestroyed fires and emits on the partially-destroyed
    // GameScene, causing a segfault.
    for (auto *item : items()) {
        if (auto *arrow = qgraphicsitem_cast<ArrowItem *>(item)) {
            delete arrow;
        }
    }

    // DO NOT call clearViews() here
    // clearViews calls close() on the zoneViews, which sends signals; sending signals in destructors leads to segfaults
    // deleteLater() deletes the zoneView without allowing it to send signals
    for (const auto &zoneView : zoneViews) {
        zoneView->deleteLater();
    }
}

/**
 * @brief Updates localized text in all zone views.
 */
void GameScene::retranslateUi()
{
    for (ZoneViewWidget *view : zoneViews) {
        view->retranslateUi();
    }
}

QList<CardState *> GameScene::selectedCardStates() const
{
    QList<CardState *> selectedStates;
    for (auto *card : selectedCards()) {
        selectedStates.append(card->getState());
    }
    return selectedStates;
}

QList<CardItem *> GameScene::selectedCards() const
{
    QList<CardItem *> selectedCards;
    for (auto item : selectedItems()) {
        if (auto card = qgraphicsitem_cast<CardItem *>(item)) {
            selectedCards.append(card);
        }
    }

    return selectedCards;
}

/**
 * @brief Selects every card of @p zone for which @p filter holds.
 * @param filter A predicate to filter which cards are selected.
 */
static void selectCardsInZone(
    const CardZoneLogic *zone,
    std::function<bool(const CardItem *)> filter = [](const CardItem *) { return true; })
{
    if (!zone) {
        return;
    }

    for (auto *cardState : zone->getCards()) {
        auto *cardItem = cardState == nullptr ? nullptr : qobject_cast<CardItem *>(cardState->parent());
        if (cardItem && filter(cardItem)) {
            cardItem->setSelected(true);
        }
    }
}

void GameScene::selectAllInZone(const CardItem *card)
{
    if (!card) {
        return;
    }

    selectCardsInZone(card->getZone());
}

void GameScene::selectSameRow(const CardItem *card)
{
    if (!card) {
        return;
    }

    auto isSameRow = [card](const CardItem *cardItem) {
        return qAbs(card->scenePos().y() - cardItem->scenePos().y()) < 50;
    };
    selectCardsInZone(card->getZone(), isSameRow);
}

void GameScene::selectSameColumn(const CardItem *card)
{
    if (!card) {
        return;
    }

    auto isSameColumn = [card](const CardItem *cardItem) { return cardItem->x() == card->x(); };
    selectCardsInZone(card->getZone(), isSameColumn);
}

CardItem *GameScene::getActiveCard() const
{
    return activeCard;
}

void GameScene::setActiveCard(CardItem *card)
{
    activeCard = card;
}

void GameScene::onCardSelectionChanged(AbstractCardItem *abstractCard, bool selected)
{
    CardItem *card = qobject_cast<CardItem *>(abstractCard);
    if (!card || !card->getOwner()) {
        return;
    }

    auto *owner = card->getOwner();

    if (selected) {
        owner->requestCardMenuUpdate(card->getState());
        return;
    }

    if (selectedItems().isEmpty()) {
        setActiveCard(nullptr);
        owner->requestCardMenuUpdate(nullptr);
    }
}

void GameScene::onCardRightClicked(AbstractCardItem *abstractCard, QPoint screenPos)
{
    auto *card = qobject_cast<CardItem *>(abstractCard);
    if (!card) {
        return;
    }
    if (!card->getOwner()) {
        return;
    }
    auto *view = playerViews.value(card->getOwner()->getPlayerInfo()->getId());
    if (!view) {
        return;
    }

    // The zoom and panel triggers are independently bindable: a right-click fires
    // whichever phases listen to it. Once right-click drives either phase the context
    // menu stays suppressed — popping it would steal the flow (and the panel already
    // carries every action when it is the panel trigger).
    auto &display = SettingsCache::instance().cardsDisplay();
    const bool zoomFired = display.getHoverZoomTrigger() == CardsDisplaySettings::HoverPanelTriggerRightClick;
    const bool panelFired = display.getHoverPanelTrigger() == CardsDisplaySettings::HoverPanelTriggerRightClick;
    if (zoomFired || panelFired) {
        fireTriggers(card, zoomFired, panelFired);
        return;
    }

    // Hide the hover popup when the context menu opens
    hideCardHoverPopup();

    setActiveCard(card);

    if (auto *menu = view->getPlayerMenu()->updateCardMenu(card->getState())) {
        menu->popup(screenPos);
    }
}

void GameScene::playSelected(AbstractCardItem *card)
{
    if (!card) {
        return;
    }
    if (!card->getOwner()) {
        return;
    }
    card->getOwner()->getPlayerActions()->actPlay(selectedCardStates());
}

void GameScene::playSelectedFaceDown(AbstractCardItem *card)
{
    if (!card) {
        return;
    }
    if (!card->getOwner()) {
        return;
    }
    card->getOwner()->getPlayerActions()->actPlayFacedown(selectedCardStates());
}

void GameScene::hideSelected(AbstractCardItem *card)
{
    if (!card) {
        return;
    }
    if (!card->getOwner()) {
        return;
    }
    card->getOwner()->getPlayerActions()->actHide(selectedCardStates());
}

/**
 * @brief Adds a player to the scene and stores their graphics item.
 * @param player Player to add.
 *
 * Connects to the player's sizeChanged signal to recompute layout on resize.
 */
void GameScene::addPlayer(PlayerLogic *player)
{
    qCInfo(GameScenePlayerAdditionRemovalLog) << "GameScene::addPlayer name=" << player->getPlayerInfo()->getName();

    auto *view = new PlayerGraphicsItem(player);

    playerViews.insert(player->getPlayerInfo()->getId(), view);
    addItem(view);
    connect(view, &PlayerGraphicsItem::sizeChanged, this, &GameScene::rearrange);

    connect(player, &PlayerLogic::concededChanged, this, [this](int id, bool conceded) {
        if (conceded) {
            clearArrowsForPlayer(id);
        }
        rearrange();
    });

    connect(player, &PlayerLogic::requestZoneViewToggle, this, &GameScene::toggleZoneView);
    connect(player, &PlayerLogic::requestRevealedZoneView, this, &GameScene::addRevealedZoneView);
    connect(player, &PlayerLogic::arrowDeleted, this, &GameScene::deleteArrow);
    connect(player, &PlayerLogic::arrowCreateRequested, this, &GameScene::addArrow);
    connect(player, &PlayerLogic::arrowDeleteRequested, this, &GameScene::requestArrowDeletion);
    connect(player, &PlayerLogic::arrowsClearedLocally, this,
            [this, id = player->getPlayerInfo()->getId()]() { clearArrowsForPlayerLocally(id); });

    connect(player->getPlayerEventHandler(), &PlayerEventHandler::cardZoneChanged, this, &GameScene::onCardZoneChanged);
    connect(player->getPlayerEventHandler(), &PlayerEventHandler::cardViewRefreshRequested, this,
            &GameScene::onCardViewRefreshRequested);

    rearrange();
}

/**
 * @brief Removes a player from the scene.
 * @param player Player to remove.
 *
 * Closes any zone views associated with the player and recomputes layout.
 */
void GameScene::removePlayer(PlayerLogic *player)
{
    qCInfo(GameScenePlayerAdditionRemovalLog) << "GameScene::removePlayer name=" << player->getPlayerInfo()->getName();

    clearArrowsForPlayer(player->getPlayerInfo()->getId());

    // Closing a view removes it from zoneViews synchronously, so iterate over a
    // copy: otherwise a player with several open views (e.g. library and hand)
    // only has the first one closed here and the remaining views are left
    // pointing at a player that is about to be deleted.
    const QList<ZoneViewWidget *> zoneViewCopy = zoneViews;
    for (ZoneViewWidget *zone : zoneViewCopy) {
        if (zone->getPlayer() == player) {
            zone->close();
        }
    }
    auto *view = playerViews.take(player->getPlayerInfo()->getId());
    removeItem(view);
    view->deleteLater();
    rearrange();
}

/**
 * @brief Adjusts the global rotation offset for player layout.
 * @param rotationAdjustment Number of positions to rotate.
 *
 * Recomputes player layout after applying rotation.
 */
void GameScene::adjustPlayerRotation(int rotationAdjustment)
{
    playerRotation += rotationAdjustment;
    rearrange();
}

/**
 * @brief Recomputes the layout of players and the scene size.
 *
 * Steps:
 * 1. Collect active players who haven't conceded.
 * 2. Rotate player list based on first local player and rotation offset.
 * 3. Determine number of columns.
 * 4. Compute scene size and layout.
 * 5. Update toolbar height and scene rectangle.
 * 6. Adjust columns and player positions to match view size.
 */
void GameScene::rearrange()
{
    if (rearranging) {
        needsReArrange = true;
        return;
    }
    rearranging = true;
    do {
        needsReArrange = false;

        int firstPlayerIndex = 0;
        auto playersPlaying = collectActivePlayers(firstPlayerIndex);
        playersPlaying = rotatePlayers(playersPlaying, firstPlayerIndex);

        int columns = determineColumnCount(playersPlaying.size());
        QSizeF sceneSize = computeSceneSizeAndPlayerLayout(playersPlaying, columns);

        phasesToolbar->setHeight(sceneSize.height());
        setSceneRect(0, 0, sceneSize.width(), sceneSize.height());

        processViewSizeChange(viewSize);
    } while (needsReArrange);
    rearranging = false;
}

// ---------- View Size ----------

/**
 * @brief Handles view resize and redistributes player positions.
 * @param newSize New view size.
 *
 * Steps:
 * 1. Compute minimum width per column from player items.
 * 2. Determine new scene width respecting aspect ratio.
 * 3. Resize columns and reposition players proportionally.
 */
void GameScene::processViewSizeChange(const QSize &newSize)
{
    viewSize = newSize;

    QList<qreal> minWidthByColumn = calculateMinWidthByColumn();
    qreal minWidth = std::accumulate(minWidthByColumn.begin(), minWidthByColumn.end(), phasesToolbar->getWidth());

    qreal newWidth = calculateNewSceneWidth(newSize, minWidth);
    setSceneRect(0, 0, newWidth, sceneRect().height());

    resizeColumnsAndPlayers(minWidthByColumn, newWidth);
}

// ---------- Player Layout Helpers ----------

/**
 * @brief Collects all active (non-conceded) players.
 * @param firstPlayerIndex Output index of first local player.
 * @return List of active players.
 *
 * Used to determine rotation and layout order.
 */
QList<PlayerLogic *> GameScene::collectActivePlayers(int &firstPlayerIndex) const
{
    QList<PlayerLogic *> activePlayers;
    firstPlayerIndex = 0;
    bool firstPlayerFound = false;

    for (auto *pgItem : playerViews.values()) {
        PlayerLogic *p = pgItem->getLogic();
        if (p && !p->getConceded()) {
            activePlayers.append(p);
            if (!firstPlayerFound && p->getPlayerInfo()->getLocal()) {
                firstPlayerIndex = activePlayers.size() - 1;
                firstPlayerFound = true;
            }
        }
    }
    return activePlayers;
}

/**
 * @brief Rotates the list of players for layout.
 * @param players Original list of players.
 * @param firstPlayerIndex Index of first local player.
 * @return Rotated list.
 *
 * Applies rotation offset and ensures the list wraps correctly.
 */
QList<PlayerLogic *> GameScene::rotatePlayers(const QList<PlayerLogic *> &activePlayers, int firstPlayerIndex) const
{
    QList<PlayerLogic *> rotated = activePlayers;
    if (!rotated.isEmpty()) {
        int totalRotation = firstPlayerIndex + playerRotation;
        while (totalRotation < 0) {
            totalRotation += rotated.size();
        }
        for (int i = 0; i < totalRotation; ++i) {
            rotated.append(rotated.takeFirst());
        }
    }
    return rotated;
}

int GameScene::determineColumnCount(int playerCount)
{
    return playerCount < SettingsCache::instance().userInterface().getMinPlayersForMultiColumnLayout() ? 1 : 2;
}

/**
 * @brief Computes layout positions and scene size based on players and columns.
 * @param playersPlaying List of active players.
 * @param columns Number of columns to split into.
 * @return Calculated scene size.
 *
 * Logic:
 * - Determine rows per column (rounding up).
 * - Calculate column widths based on widest player item.
 * - Accumulate scene width and height.
 * - Position players in columns with spacing.
 * - Mirror graphics for visual balance.
 */
QSizeF GameScene::computeSceneSizeAndPlayerLayout(const QList<PlayerLogic *> &playersPlaying, int columns)
{
    playersByColumn.clear();

    int rows = qCeil((qreal)playersPlaying.size() / columns);
    qreal sceneHeight = 0, sceneWidth = -playerAreaSpacing;
    QList<int> columnWidth;

    QListIterator<PlayerLogic *> playersIter(playersPlaying);
    for (int col = 0; col < columns; ++col) {
        playersByColumn.append(QList<PlayerGraphicsItem *>());
        columnWidth.append(0);
        qreal thisColumnHeight = -playerAreaSpacing;
        int rowsInColumn = rows - (playersPlaying.size() % columns) * col; // Adjust rows for uneven columns

        for (int j = 0; j < rowsInColumn; ++j) {
            PlayerLogic *player = playersIter.next();
            if (col == 0) {
                playersByColumn[col].prepend(playerViews.value(player->getPlayerInfo()->getId()));
            } else {
                playersByColumn[col].append(playerViews.value(player->getPlayerInfo()->getId()));
            }

            auto *pgItem = playerViews.value(player->getPlayerInfo()->getId());
            thisColumnHeight += pgItem->boundingRect().height() + playerAreaSpacing;
            columnWidth[col] = std::max(columnWidth[col], (int)pgItem->boundingRect().width());
        }

        sceneHeight = std::max(sceneHeight, thisColumnHeight);
        sceneWidth += columnWidth[col] + playerAreaSpacing;
    }

    qreal phasesWidth = phasesToolbar->getWidth();
    sceneWidth += phasesWidth;

    // Position players horizontally and vertically
    qreal x = phasesWidth;
    for (int col = 0; col < columns; ++col) {
        qreal y = 0;
        for (int row = 0; row < playersByColumn[col].size(); ++row) {
            PlayerGraphicsItem *player = playersByColumn[col][row];
            player->setPos(x, y);
            player->setMirrored(row != rows - 1); // Mirror all except bottom-most
            y += player->boundingRect().height() + playerAreaSpacing;
        }
        x += columnWidth[col] + playerAreaSpacing;
    }

    return QSizeF(sceneWidth, sceneHeight);
}

/**
 * @brief Computes the minimum width for each column based on player minimum widths.
 * @return List of minimum widths per column.
 */
QList<qreal> GameScene::calculateMinWidthByColumn() const
{
    QList<qreal> minWidthByColumn;
    for (const auto &col : playersByColumn) {
        qreal maxWidth = 0;
        for (PlayerGraphicsItem *player : col) {
            maxWidth = std::max(maxWidth, player->getMinimumWidth());
        }
        minWidthByColumn.append(maxWidth);
    }
    return minWidthByColumn;
}

/**
 * @brief Calculates new scene width considering window aspect ratio.
 * @param newSize View size.
 * @param minWidth Minimum width needed to fit all players.
 * @return Scene width respecting window and content.
 */
qreal GameScene::calculateNewSceneWidth(const QSize &newSize, qreal minWidth) const
{
    qreal newRatio = (qreal)newSize.width() / newSize.height();
    qreal minRatio = minWidth / sceneRect().height();

    if (minRatio > newRatio) {
        return minWidth; // Table dominates width
    } else {
        return newRatio * sceneRect().height(); // Window ratio dominates
    }
}

/**
 * @brief Resizes columns and distributes extra width to players.
 * @param minWidthByColumn Minimum widths per column.
 * @param newWidth Total scene width.
 *
 * Extra width is distributed evenly across columns. Each player item is
 * notified to adjust internal layout for the new column width.
 */
void GameScene::resizeColumnsAndPlayers(const QList<qreal> &minWidthByColumn, qreal newWidth)
{
    qreal minWidth = std::accumulate(minWidthByColumn.begin(), minWidthByColumn.end(), phasesToolbar->getWidth());

    qreal extraWidthPerColumn = (newWidth - minWidth) / playersByColumn.size();
    qreal newx = phasesToolbar->getWidth();

    // Snapshot the columns: resizing a player's table can synchronously trigger
    // GameScene::rearrange (table width -> sizeChanged -> updateBoundingRect ->
    // sizeChanged -> rearrange), and rearrange rebuilds playersByColumn. Iterating
    // the live container across that re-entrant call would use invalidated iterators.
    const QList<QList<PlayerGraphicsItem *>> columns = playersByColumn;

    for (int col = 0; col < columns.size(); ++col) {
        for (PlayerGraphicsItem *player : columns[col]) {
            player->processSceneSizeChange(minWidthByColumn[col] + extraWidthPerColumn);
            player->setPos(newx, player->y());
        }
        newx += minWidthByColumn[col] + extraWidthPerColumn;
    }
}

void GameScene::addArrow(QSharedPointer<ArrowData> data)
{
    auto *startView = playerViews.value(data->startPlayerId);
    auto *targetView = playerViews.value(data->targetPlayerId);
    if (!startView || !targetView) {
        return;
    }

    PlayerLogic *startLogic = startView->getLogic();
    auto *startZone = startLogic->getZones().value(data->startZone);
    if (!startZone) {
        return;
    }

    CardState *startCardState = startZone->getCard(data->startCardId);
    if (!startCardState) {
        return;
    }
    CardItem *startCard = qobject_cast<CardItem *>(startCardState->parent());
    if (!startCard) {
        return;
    }

    ArrowTarget *targetItem = nullptr;
    if (data->isPlayerTargeted()) {
        targetItem = targetView->getPlayerTarget();
    } else {
        auto *zone = targetView->getLogic()->getZones().value(data->targetZone);
        if (zone) {
            CardState *targetCardState = zone->getCard(data->targetCardId);
            targetItem = targetCardState == nullptr ? nullptr : qobject_cast<CardItem *>(targetCardState->parent());
        }
    }
    if (!targetItem) {
        return;
    }

    auto *arrow = new ArrowItem(data, startCard, targetItem);
    addItem(arrow);
    arrow->startDrawAnimation();
    arrowRegistry.insert(data, arrow);
    connect(arrow, &ArrowItem::requestDeletion, this, &GameScene::requestArrowDeletion);
}

void GameScene::deleteArrow(int playerId, int arrowId)
{
    if (auto *arrow = arrowRegistry.take(playerId, arrowId)) {
        arrow->delArrow();
    }
}

void GameScene::requestArrowDeletion(int playerId, int arrowId)
{
    if (arrowRegistry.contains(playerId, arrowId)) {
        emit arrowDeletionRequested(playerId, arrowId);
    }
}
void GameScene::onCardZoneChanged(CardState *card, bool sameZone)
{
    auto *item = qobject_cast<CardItem *>(card->parent());
    if (!item) {
        return;
    }
    QList<ArrowItem *> toDelete;
    for (auto *arrow : arrowRegistry.all()) {
        if (arrow->getStartItem() == item || arrow->getTargetItem() == item) {
            if (sameZone) {
                arrow->updatePath();
            } else {
                toDelete.append(arrow);
            }
        }
    }
    for (auto *arrow : toDelete) {
        deleteArrow(arrow->getCreatorId(), arrow->getId());
    }

    // The hover panel stays open on a moved card: follow it instead of stranding.
    if (!sameZone && cardHoverWidget && cardHoverWidget->getCardItem() == item) {
        positionCardHoverPopup();
    }
}

void GameScene::onCardViewRefreshRequested(CardState *card, bool zoneChanged, PlayerLogic *newOwner)
{
    auto *item = qobject_cast<CardItem *>(card->parent());
    if (!item) {
        return;
    }
    if (zoneChanged) {
        item->deleteCardInfoPopup();
    }
    item->deleteDragItem();
    if (zoneChanged) {
        item->setBeingPointedAt(false);
        item->setHovered(false);
    }
    if (newOwner) {
        item->setOwner(newOwner);
    }
}

void GameScene::clearArrowsForPlayer(int playerId)
{
    for (int arrowId : arrowRegistry.idsForPlayer(playerId)) {
        emit requestArrowDeletion(playerId, arrowId);
    }
}

void GameScene::clearArrowsForPlayerLocally(int playerId)
{
    for (int arrowId : arrowRegistry.idsForPlayer(playerId)) {
        if (auto *arrow = arrowRegistry.take(playerId, arrowId)) {
            arrow->delArrow();
        }
    }
}
// ---------- Hover Handling ----------

CardItem *GameScene::getHoveredCard() const
{
    return hoveredCard;
}

void GameScene::updateHover(const QPointF &scenePos)
{
    updateHoveredCard(pickCardAt(scenePos));
}

CardItem *GameScene::pickCardAt(const QPointF &scenePos)
{
    auto itemList = items(scenePos, Qt::IntersectsItemBoundingRect, Qt::DescendingOrder, getViewTransform());
    CardZone *zone = findTopmostZone(itemList);
    CardItem *topCard = zone ? findTopmostCardInZone(itemList, zone) : nullptr;
    if (!topCard && hoveredCard) {
        const QRectF guard = hoveredCard->sceneBoundingRect().adjusted(-8, -8, 8, 8);
        if (guard.contains(scenePos)) {
            // The zoomed card overflows onto neighboring zones, where naive topmost-zone
            // picking attributes the cursor to the zone underneath. While the cursor is
            // still over the zoomed footprint (with hysteresis for edge jiggle), keep it.
            // A genuinely different topmost card still takes over.
            return hoveredCard;
        }
    }
    return topCard;
}

void GameScene::updateHoveredCard(CardItem *newCard)
{
    // Safety: if the popup's card was destroyed, hide the popup immediately
    if (cardHoverWidget && cardHoverWidget->isVisible() && !cardHoverWidget->hasCard()) {
        hideCardHoverPopup();
    }
    // The hovered card may have been destroyed without ending the hover: its QPointer
    // nulled itself, but the ancestor lift is still outstanding. Restore it now.
    if (!hoveredCard && !hoverLiftedAncestors.isEmpty()) {
        restoreHoverLift();
    }

    // Hovering alone never starts anything anymore: the trigger (dwell, click, wheel,
    // shortcut) resolves first, then the zoom follows. This keeps the feature calm.
    bool wasHoveringCard = (hoveredCard != nullptr);
    if (hoveredCard && (newCard != hoveredCard)) {
        endCardHover(hoveredCard);
        hoveredCard = nullptr;
    }

    auto &display = SettingsCache::instance().cardsDisplay();
    const int zoomTrigger = display.getHoverZoomTrigger();
    const int panelTrigger = display.getHoverPanelTrigger();
    const bool doubleConfirm = display.getHoverPanelDoubleConfirm() && zoomTrigger == panelTrigger;

    if (newCard) {
        // Cancel any pending grace timer (mouse is back on a card)
        hoverGracePending = false;
        hoverGraceTimer->stop();

        if (newCard == hoveredCard) {
            // Steady state: zoom already up. Refresh an open panel, otherwise nothing —
            // in particular this must not re-fire an armed double-confirm.
            if (cardHoverWidget && cardHoverWidget->isVisible() && cardHoverWidget->getCardItem() == newCard) {
                cardHoverWidget->refreshDisplay();
            }
            return;
        }
        if (cardHoverWidget && cardHoverWidget->isVisible() && cardHoverWidget->getCardItem() == newCard) {
            // Panel open but zoom dropped (e.g. survived a Leave) — zoom back and refresh.
            beginCardHover(newCard);
            hoveredCard = newCard;
            cardHoverWidget->refreshDisplay();
            return;
        }
        if (cardHoverWidget && cardHoverWidget->isVisible()) {
            // Panel open elsewhere — move the zoom at once so it never strands, but let
            // the panel follow after the configured delay instead of strobing open on
            // every card swept across.
            beginCardHover(newCard);
            hoveredCard = newCard;
            confirmArmedCard.clear();
            hoverPanelTarget = newCard;
            hoverPanelTimer->start(display.getHoverPanelDelayMs());
            return;
        }
        if (zoomTrigger != CardsDisplaySettings::HoverPanelTriggerHover &&
            panelTrigger != CardsDisplaySettings::HoverPanelTriggerHover) {
            // Mouse contact alone drives neither phase in this config.
            return;
        }
        if (doubleConfirm && confirmArmedCard == newCard) {
            // Repeat hover on an armed card: zoom and show at once, no second dwell.
            beginCardHover(newCard);
            hoveredCard = newCard;
            confirmArmedCard.clear();
            showCardHoverPopup(newCard);
            return;
        }
        if (newCard != hoverPopupTarget) {
            // First contact (or a new card): dwell, then resolve on timeout.
            hoverPopupTarget = newCard;
            hoverActivateTimer->start(display.getHoverZoomDelayMs());
        }
    } else {
        // No card hovered: drop any pending dwell/panel timers (kills stale shows) and start
        // the grace timer only when the mouse just left a card with the panel open.
        hoverPopupTarget = nullptr;
        hoverPanelTarget = nullptr;
        hoverActivateTimer->stop();
        hoverPanelTimer->stop();
        // Non-hover zoom arms only make sense while approaching the card: leaving
        // re-arms the zoom for the next explicit trigger instead of showing the panel.
        // (Hover-trigger arms survive the leave so re-entering completes the repeat.)
        if (zoomTrigger != CardsDisplaySettings::HoverPanelTriggerHover) {
            confirmArmedCard.clear();
        }
        if (wasHoveringCard && cardHoverWidget && cardHoverWidget->isVisible() &&
            !cardHoverWidget->isMouseOverWidget() && !hoverGracePending) {
            hoverGracePending = true;
            hoverGraceTimer->start();
        }
    }
}

void GameScene::beginCardHover(CardItem *card)
{
    card->setHovered(true);
    if (auto *zone = SelectZone::findOwningSelectZone(card)) {
        zone->escapeClipForHover(card);
    }
    // Lift the whole ancestor chain above their siblings: the zoomed card overflows its
    // own zone, and Qt orders whole subtrees level by level, so no single z-value wins
    // every comparison on its own. Harmless elsewhere (areas and zones tile without
    // overlapping; arrows, zone views and drag items keep their higher layers).
    // Restore first: if a previous hover never ended (its card was destroyed mid-hover),
    // its chain is still outstanding and must land before a new one is recorded.
    restoreHoverLift();
    QGraphicsItem *ancestor = card->parentItem();
    while (ancestor) {
        // The chain is QGraphicsObject-derived in practice; a plain QGraphicsItem
        // ancestor cannot be guarded, so it keeps its z-value instead of risking an
        // unrestorable lift.
        if (QGraphicsObject *ancestorObject = ancestor->toGraphicsObject()) {
            hoverLiftedAncestors.append({QPointer<QGraphicsObject>(ancestorObject), ancestor->zValue()});
            ancestor->setZValue(ZValues::HOVERED_AREA);
        }
        ancestor = ancestor->parentItem();
    }
    liftOwner = card;
    connect(card, &AbstractCardItem::hoverShrinkFinished, this, &GameScene::onHoverShrinkFinished,
            Qt::UniqueConnection);
}

void GameScene::endCardHover(CardItem *card)
{
    // Only the zoom-out starts here: the z-value, the clip escape and the ancestor
    // lift stay put until the shrink finishes (onHoverShrinkFinished), so siblings and
    // the clip container cannot cut through the still-enlarged card.
    card->setHovered(false);
}

void GameScene::onHoverShrinkFinished(AbstractCardItem *card)
{
    auto *cardItem = qobject_cast<CardItem *>(card);
    if (!cardItem) {
        return;
    }
    // Per-card and idempotent: always settle, even if a newer hover owns the lift.
    if (auto *zone = SelectZone::findOwningSelectZone(cardItem)) {
        zone->restoreClipAfterHover(cardItem);
    }
    if (liftOwner != cardItem) {
        // A newer hover already owns the lift (or nothing does): leave it alone.
        return;
    }
    restoreHoverLift();
}

void GameScene::restoreHoverLift()
{
    for (const auto &[lifted, zValue] : hoverLiftedAncestors) {
        if (lifted) {
            lifted->setZValue(zValue);
        }
    }
    hoverLiftedAncestors.clear();
    liftOwner.clear();
}

// ---------- Card Hover Popup ----------

void GameScene::showCardHoverPopup(CardItem *card)
{
    if (!card || !card->getOwner()) {
        return;
    }

    PlayerGraphicsItem *playerView = playerViews.value(card->getOwner()->getPlayerInfo()->getId());
    if (!playerView) {
        return;
    }

    hoverActivateTimer->stop();
    hoverPanelTimer->stop();
    hoverGracePending = false;
    hoverGraceTimer->stop();
    hoverPopupTarget = card;
    hoverPanelTarget = nullptr;
    confirmArmedCard.clear();

    if (!cardHoverWidget) {
        auto *viewport = views().isEmpty() ? nullptr : views().first()->viewport();
        cardHoverWidget = new CardHoverWidget(viewport);
        connect(cardHoverWidget, &CardHoverWidget::hoverEntered, this, &GameScene::onHoverWidgetEntered);
        connect(cardHoverWidget, &CardHoverWidget::hoverLeft, this, &GameScene::onHoverWidgetLeft);

        // Keep the popup anchored to its card while the view scrolls, zooms, or resizes
        if (!views().isEmpty()) {
            auto *view = views().first();
            connect(view->horizontalScrollBar(), &QScrollBar::valueChanged, this, &GameScene::positionCardHoverPopup);
            connect(view->verticalScrollBar(), &QScrollBar::valueChanged, this, &GameScene::positionCardHoverPopup);
            if (viewport) {
                viewport->installEventFilter(this);
            }
        }
    }

    // Crossfade: the opaque panel picture sits exactly over the zoomed card, so fade
    // the board card out underneath it (masks any residual size/corner mismatch) and
    // fade any previously shown card back in.
    CardItem *oldCard = cardHoverWidget->hasCard() ? cardHoverWidget->getCardItem() : nullptr;

    cardHoverWidget->setCard(card, playerView);
    positionCardHoverPopup();
    cardHoverWidget->showAnimated();

    if (oldCard && oldCard != card) {
        oldCard->fadeCardIn();
    }
    card->fadeCardOut();
}

void GameScene::hideCardHoverPopup()
{
    // Hiding is terminal: drop all pending hover timing with it, or a dwell/panel
    // timer that resolves afterwards pops the panel back up (e.g. over the context
    // menu that hideCardHoverPopup was just called for).
    hoverActivateTimer->stop();
    hoverPanelTimer->stop();
    hoverGracePending = false;
    hoverGraceTimer->stop();
    if (cardHoverWidget) {
        // The panel is gone: whatever card it covered must come back.
        if (CardItem *coveredCard = cardHoverWidget->getCardItem()) {
            coveredCard->fadeCardIn();
        }
        if (cardHoverWidget->isVisible()) {
            cardHoverWidget->hideAnimated();
        }
    }
    hoverPopupTarget = nullptr;
    hoverPanelTarget = nullptr;
}

void GameScene::positionCardHoverPopup()
{
    if (!cardHoverWidget || !cardHoverWidget->hasCard()) {
        return;
    }

    auto *viewport = views().isEmpty() ? nullptr : views().first()->viewport();
    if (!viewport) {
        return;
    }

    QRectF sceneRect = cardHoverWidget->getCardItem()->sceneBoundingRect();
    QTransform vpTransform = getViewportTransform();
    cardHoverWidget->positionNextTo(sceneRect, vpTransform, viewport);
}

void GameScene::onHoverActivateTimeout()
{
    // Dwell resolved: the hover drives whichever phases listen to it. The cursor must
    // still be over the pending card (rect test, not a fresh pick, so pick flakiness
    // cannot veto it); otherwise this was a stale timer.
    CardItem *target = hoverPopupTarget;
    hoverPopupTarget = nullptr;
    if (!target || views().isEmpty()) {
        return;
    }
    auto *view = views().first();
    const QPointF cursorScene = view->mapToScene(view->viewport()->mapFromGlobal(QCursor::pos()));
    if (!target->sceneBoundingRect().contains(cursorScene)) {
        return;
    }
    auto &display = SettingsCache::instance().cardsDisplay();
    const bool zoomPhase = display.getHoverZoomTrigger() == CardsDisplaySettings::HoverPanelTriggerHover;
    const bool panelPhase = display.getHoverPanelTrigger() == CardsDisplaySettings::HoverPanelTriggerHover;
    const bool effectiveDouble =
        display.getHoverPanelDoubleConfirm() && display.getHoverZoomTrigger() == display.getHoverPanelTrigger();
    if (hoveredCard && hoveredCard != target) {
        endCardHover(hoveredCard);
    }
    if (zoomPhase) {
        if (hoveredCard != target) {
            beginCardHover(target);
            hoveredCard = target;
        }
        if (effectiveDouble) {
            confirmArmedCard = target;
        }
    }
    if (panelPhase && !effectiveDouble) {
        hoverPanelTarget = target;
        hoverPanelTimer->start(display.getHoverPanelDelayMs());
    }
}

void GameScene::onHoverPanelTimeout()
{
    // Panel delay resolved: show, but only if the target card is still the one under
    // the cursor (same cheap rect check — no dependence on pick stability).
    CardItem *target = hoverPanelTarget;
    hoverPanelTarget = nullptr;
    if (!target || views().isEmpty()) {
        return;
    }
    auto *view = views().first();
    const QPointF cursorScene = view->mapToScene(view->viewport()->mapFromGlobal(QCursor::pos()));
    if (!target->sceneBoundingRect().contains(cursorScene)) {
        return;
    }
    ensureZoomed(target);
    showCardHoverPopup(target);
}

void GameScene::onHoverGraceTimeout()
{
    hoverGracePending = false;
    if (cardHoverWidget && cardHoverWidget->isVisible() && cardHoverWidget->isMouseOverWidget()) {
        return;
    }
    hideCardHoverPopup();
}

void GameScene::summonHoverPanel()
{
    // Summon for whatever is under the cursor right now; hovering is not required.
    if (views().isEmpty()) {
        return;
    }
    auto *view = views().first();
    const QPointF cursorScene = view->mapToScene(view->viewport()->mapFromGlobal(QCursor::pos()));
    if (CardItem *card = pickCardAt(cursorScene)) {
        auto &display = SettingsCache::instance().cardsDisplay();
        fireTriggers(card, display.getHoverZoomTrigger() == CardsDisplaySettings::HoverPanelTriggerShortcut,
                     display.getHoverPanelTrigger() == CardsDisplaySettings::HoverPanelTriggerShortcut);
    }
}

void GameScene::ensureZoomed(CardItem *card)
{
    if (hoveredCard != card) {
        if (hoveredCard) {
            endCardHover(hoveredCard);
        }
        beginCardHover(card);
        hoveredCard = card;
    }
}

void GameScene::fireTriggers(CardItem *card, bool zoomFired, bool panelFired)
{
    if (!card || !card->getOwner()) {
        return;
    }

    if (cardHoverWidget && cardHoverWidget->isVisible() && cardHoverWidget->getCardItem() == card) {
        cardHoverWidget->refreshDisplay();
        return;
    }

    // Explicit triggers preempt any pending hover timing.
    hoverPopupTarget = nullptr;
    hoverPanelTarget = nullptr;
    hoverActivateTimer->stop();
    hoverPanelTimer->stop();

    auto &display = SettingsCache::instance().cardsDisplay();
    const bool effectiveDouble =
        display.getHoverPanelDoubleConfirm() && display.getHoverZoomTrigger() == display.getHoverPanelTrigger();

    if (zoomFired && panelFired) {
        // Both phases on one action: one-step zooms and shows at once (the crossfade
        // covers it); double-confirm arms on first fire and shows on repeat.
        if (effectiveDouble) {
            if (confirmArmedCard == card) {
                confirmArmedCard.clear();
                ensureZoomed(card);
                showCardHoverPopup(card);
            } else {
                confirmArmedCard = card;
                ensureZoomed(card);
            }
        } else {
            ensureZoomed(card);
            showCardHoverPopup(card);
        }
        return;
    }
    if (zoomFired) {
        ensureZoomed(card);
    }
    if (panelFired) {
        ensureZoomed(card);
        showCardHoverPopup(card);
    }
}

void GameScene::onHoverWidgetEntered()
{
    hoverGracePending = false;
    hoverGraceTimer->stop();
}

void GameScene::onHoverWidgetLeft()
{
    if (!hoveredCard && cardHoverWidget && cardHoverWidget->isVisible() && !hoverGracePending) {
        hoverGracePending = true;
        hoverGraceTimer->start();
    }
}

bool GameScene::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        positionCardHoverPopup();
    }
    return QGraphicsScene::eventFilter(watched, event);
}

CardZone *GameScene::findTopmostZone(const QList<QGraphicsItem *> &items)
{
    for (QGraphicsItem *item : items) {
        if (auto *zone = qgraphicsitem_cast<CardZone *>(item)) {
            return zone;
        }
    }
    return nullptr;
}

CardItem *GameScene::findTopmostCardInZone(const QList<QGraphicsItem *> &items, CardZone *zone)
{
    CardItem *maxZCard = nullptr;
    qreal maxZ = -1;

    for (QGraphicsItem *item : items) {
        CardItem *card = qgraphicsitem_cast<CardItem *>(item);
        if (!card) {
            continue;
        }

        if (card->getAttachedTo()) {
            if (card->getAttachedTo()->getZone() != zone->getLogic()) {
                continue;
            }
        } else if (card->getZone() != zone->getLogic()) {
            continue;
        }

        if (card->getRealZValue() > maxZ) {
            maxZ = card->getRealZValue();
            maxZCard = card;
        }
    }
    return maxZCard;
}

// ---------- Zone Views ----------

/**
 * @brief Toggles a zone view for a player.
 * @param player Player owning the zone.
 * @param zoneName Name of the zone.
 * @param numberCards Number of cards visible in the view.
 * @param isReversed Whether the zone view is reversed.
 *
 * If an identical view exists, it is closed. Otherwise, a new ZoneViewWidget is created
 * and positioned based on zone type.
 */
void GameScene::toggleZoneView(PlayerLogic *player, const QString &zoneName, int numberCards, bool isReversed)
{
    // Closing a view removes it from zoneViews synchronously, so iterate over a
    // copy to make sure every already-open matching view is closed.
    const QList<ZoneViewWidget *> zoneViewCopy = zoneViews;
    for (auto *view : zoneViewCopy) {
        ZoneViewZone *temp = view->getZone();
        if (temp->getLogic()->getName() == zoneName && temp->getLogic()->getPlayer() == player &&
            qobject_cast<ZoneViewZoneLogic *>(temp->getLogic())->getNumberCards() == numberCards) {
            view->close();
        }
    }

    ZoneViewWidget *item =
        new ZoneViewWidget(player, player->getZones().value(zoneName), numberCards, false, false, {}, isReversed);

    zoneViews.append(item);
    connect(item, &ZoneViewWidget::closePressed, this, &GameScene::removeZoneView);
    addItem(item);

    if (zoneName == ZoneNames::GRAVE) {
        item->setPos(360, 100);
    } else if (zoneName == ZoneNames::EXILE) {
        item->setPos(380, 120);
    } else {
        item->setPos(340, 80);
    }
}

/**
 * @brief Adds a revealed zone view (for shown cards).
 * @param player Owning player.
 * @param zone Zone logic.
 * @param cardList List of cards to show.
 * @param withWritePermission Whether edits are allowed.
 */
void GameScene::addRevealedZoneView(PlayerLogic *player,
                                    CardZoneLogic *zone,
                                    const QList<const ServerInfo_Card *> &cardList,
                                    bool withWritePermission)
{
    ZoneViewWidget *item = new ZoneViewWidget(player, zone, -2, true, withWritePermission, cardList);
    zoneViews.append(item);
    connect(item, &ZoneViewWidget::closePressed, this, &GameScene::removeZoneView);
    addItem(item);
    item->setPos(600, 80);
}

/**
 * @brief Removes a zone view widget from the scene.
 * @param item Zone view to remove.
 */
void GameScene::removeZoneView(ZoneViewWidget *item)
{
    zoneViews.removeOne(item);
    removeItem(item);
}

/**
 * @brief Closes all zone views.
 */
void GameScene::clearViews()
{
    while (!zoneViews.isEmpty()) {
        zoneViews.first()->close();
    }
}

void GameScene::emptyActiveViews()
{
    for (auto zoneView : zoneViews) {
        auto logic = qobject_cast<ZoneViewZoneLogic *>(zoneView->getZone()->getLogic());
        if (!logic->getRevealZone()) {
            logic->clearCards();
        }
    }
}

/**
 * @brief Closes the most recently added zone view.
 */
void GameScene::closeMostRecentZoneView()
{
    if (!zoneViews.isEmpty()) {
        zoneViews.last()->close();
    }
}

// ---------- View Transforms ----------

QTransform GameScene::getViewTransform() const
{
    return views().at(0)->transform();
}

QTransform GameScene::getViewportTransform() const
{
    return views().at(0)->viewportTransform();
}

// ---------- Event Handling ----------

bool GameScene::event(QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMouseMove) {
        updateHover(static_cast<QGraphicsSceneMouseEvent *>(event)->scenePos());
    } else if (event->type() == QEvent::GraphicsSceneWheel) {
        // Wheel trigger: wheeling over a card fires whichever phases listen to it
        // instead of scrolling. Fully consumed here: forwarding to the base dispatcher
        // as well would double-deliver it to items under the cursor (e.g. zone views,
        // which scroll on wheel). Scrolling up instead reverts an active zoom/panel —
        // backing out of the gesture undoes it; with nothing to revert the event falls
        // through to normal scrolling.
        auto &display = SettingsCache::instance().cardsDisplay();
        const bool zoomFired = display.getHoverZoomTrigger() == CardsDisplaySettings::HoverPanelTriggerWheel;
        const bool panelFired = display.getHoverPanelTrigger() == CardsDisplaySettings::HoverPanelTriggerWheel;
        if (zoomFired || panelFired) {
            auto *wheelEvent = static_cast<QGraphicsSceneWheelEvent *>(event);
            // QGraphicsSceneWheelEvent carries delta()/pixelDelta(), not angleDelta().
            const int wheelUp = !wheelEvent->pixelDelta().isNull()
                                    ? wheelEvent->pixelDelta().y()
                                    : (wheelEvent->orientation() == Qt::Vertical ? wheelEvent->delta() : 0);
            if (wheelUp > 0) {
                if ((cardHoverWidget && cardHoverWidget->isVisible()) || hoveredCard) {
                    hoverPopupTarget = nullptr;
                    hoverPanelTarget = nullptr;
                    confirmArmedCard.clear();
                    hideCardHoverPopup();
                    if (hoveredCard) {
                        endCardHover(hoveredCard);
                        hoveredCard = nullptr;
                    }
                    event->accept();
                    return true;
                }
            } else if (CardItem *card = pickCardAt(wheelEvent->scenePos())) {
                fireTriggers(card, zoomFired, panelFired);
                event->accept();
                return true;
            }
        }
    } else if (event->type() == QEvent::Leave) {
        // The hover panel opens directly over the zoomed card under the cursor, which the
        // scene observes as a Leave. The cursor never actually left: if it is over the open
        // panel, keep the hover (dropping it here shrinks the card from under the panel and
        // starves the popup logic, so the menu never shows). A genuine leave of the window
        // misses the panel and still ends the hover as before.
        if (cardHoverWidget && cardHoverWidget->isVisible() && cardHoverWidget->isMouseOverWidget()) {
            return QGraphicsScene::event(event);
        }
        updateHoveredCard(nullptr);
    }

    return QGraphicsScene::event(event);
}

void GameScene::timerEvent(QTimerEvent * /*event*/)
{
    QMutableHashIterator<QObject *, IAnimatedItem *> i(animatedItems);
    while (i.hasNext()) {
        i.next();
        if (!i.value()->animationEvent()) {
            i.remove();
        }
    }
    if (animatedItems.isEmpty()) {
        animationTimer->stop();
    }
}

void GameScene::registerAnimationItem(IAnimatedItem *item)
{
    auto *object = dynamic_cast<QObject *>(item);
    if (!object) {
        return;
    }
    // Guard against duplicate connections using the connection map, not
    // animatedItems: the animation timer removes entries from animatedItems when an
    // animation completes, but the destroyed->removeAnimatedItem connection must
    // persist until the object is destroyed. Relying on animatedItems here would let
    // a re-registered item (e.g. a life counter that flashes repeatedly) accumulate
    // duplicate destroyed connections, the older ones of which would survive teardown.
    if (!animationItemConnections.contains(object)) {
        animationItemConnections.insert(object,
                                        connect(object, &QObject::destroyed, this, &GameScene::removeAnimatedItem));
    }
    animatedItems.insert(object, item);
    if (animationTimer && !animationTimer->isActive()) {
        animationTimer->start(10, this);
    }
}

void GameScene::unregisterAnimationItem(IAnimatedItem *item)
{
    animatedItems.remove(dynamic_cast<QObject *>(item));
    if (animationTimer && animatedItems.isEmpty()) {
        animationTimer->stop();
    }
}

void GameScene::removeAnimatedItem(QObject *item)
{
    animatedItems.remove(item);
    animationItemConnections.remove(item);
    if (animationTimer && animatedItems.isEmpty()) {
        animationTimer->stop();
    }
}

// ---------- Rubber Band ----------

void GameScene::startRubberBand(const QPointF &selectionOrigin)
{
    emit sigStartRubberBand(selectionOrigin);
}

void GameScene::resizeRubberBand(const QPointF &cursorPoint, int selectedCount)
{
    emit sigResizeRubberBand(cursorPoint, selectedCount);
}

void GameScene::stopRubberBand()
{
    emit sigStopRubberBand();
}
