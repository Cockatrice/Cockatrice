#include "game_scene.h"

#include "board/card_item.h"
#include "phases_toolbar.h"
#include "player/player.h"
#include "player/player_graphics_item.h"
#include "zones/view_zone.h"
#include "zones/view_zone_widget.h"

#include <QAction>
#include <QBasicTimer>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QSet>
#include <QtMath>
#include <libcockatrice/settings/cache_settings.h>
#include <numeric>

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
    connect(&SettingsCache::instance(), &SettingsCache::minPlayersForMultiColumnLayoutChanged, this,
            &GameScene::rearrange);

    rearrange();
}

GameScene::~GameScene()
{
    delete animationTimer;

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
    for (ZoneViewWidget *view : zoneViews)
        view->retranslateUi();
}

/**
 * @brief Adds a player to the scene and stores their graphics item.
 * @param player Player to add.
 *
 * Connects to the player's sizeChanged signal to recompute layout on resize.
 */
void GameScene::addPlayer(Player *player)
{
    qCInfo(GameScenePlayerAdditionRemovalLog) << "GameScene::addPlayer name=" << player->getPlayerInfo()->getName();

    players << player->getGraphicsItem();
    addItem(player->getGraphicsItem());
    connect(player->getGraphicsItem(), &PlayerGraphicsItem::sizeChanged, this, &GameScene::rearrange);
}

/**
 * @brief Removes a player from the scene.
 * @param player Player to remove.
 *
 * Closes any zone views associated with the player and recomputes layout.
 */
void GameScene::removePlayer(Player *player)
{
    qCInfo(GameScenePlayerAdditionRemovalLog) << "GameScene::removePlayer name=" << player->getPlayerInfo()->getName();

    for (ZoneViewWidget *zone : zoneViews) {
        if (zone->getPlayer() == player) {
            zone->close();
        }
    }
    players.removeOne(player->getGraphicsItem());
    removeItem(player->getGraphicsItem());
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
    int firstPlayerIndex = 0;
    auto playersPlaying = collectActivePlayers(firstPlayerIndex);
    playersPlaying = rotatePlayers(playersPlaying, firstPlayerIndex);

    int columns = determineColumnCount(playersPlaying.size());
    QSizeF sceneSize = computeSceneSizeAndPlayerLayout(playersPlaying, columns);

    phasesToolbar->setHeight(sceneSize.height());
    setSceneRect(0, 0, sceneSize.width(), sceneSize.height());

    processViewSizeChange(viewSize);
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
QList<Player *> GameScene::collectActivePlayers(int &firstPlayerIndex) const
{
    QList<Player *> activePlayers;
    firstPlayerIndex = 0;
    bool firstPlayerFound = false;

    for (auto *pgItem : players) {
        Player *p = pgItem->getPlayer();
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
QList<Player *> GameScene::rotatePlayers(const QList<Player *> &activePlayers, int firstPlayerIndex) const
{
    QList<Player *> rotated = activePlayers;
    if (!rotated.isEmpty()) {
        int totalRotation = firstPlayerIndex + playerRotation;
        while (totalRotation < 0)
            totalRotation += rotated.size();
        for (int i = 0; i < totalRotation; ++i)
            rotated.append(rotated.takeFirst());
    }
    return rotated;
}

int GameScene::determineColumnCount(int playerCount)
{
    return playerCount < SettingsCache::instance().getMinPlayersForMultiColumnLayout() ? 1 : 2;
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
QSizeF GameScene::computeSceneSizeAndPlayerLayout(const QList<Player *> &playersPlaying, int columns)
{
    playersByColumn.clear();

    int rows = qCeil((qreal)playersPlaying.size() / columns);
    qreal sceneHeight = 0, sceneWidth = -playerAreaSpacing;
    QList<int> columnWidth;

    QListIterator<Player *> playersIter(playersPlaying);
    for (int col = 0; col < columns; ++col) {
        playersByColumn.append(QList<PlayerGraphicsItem *>());
        columnWidth.append(0);
        qreal thisColumnHeight = -playerAreaSpacing;
        int rowsInColumn = rows - (playersPlaying.size() % columns) * col; // Adjust rows for uneven columns

        for (int j = 0; j < rowsInColumn; ++j) {
            Player *player = playersIter.next();
            if (col == 0)
                playersByColumn[col].prepend(player->getGraphicsItem());
            else
                playersByColumn[col].append(player->getGraphicsItem());

            auto *pgItem = player->getGraphicsItem();
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
        for (PlayerGraphicsItem *player : col)
            maxWidth = std::max(maxWidth, player->getMinimumWidth());
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

    for (int col = 0; col < playersByColumn.size(); ++col) {
        for (PlayerGraphicsItem *player : playersByColumn[col]) {
            player->processSceneSizeChange(minWidthByColumn[col] + extraWidthPerColumn);
            player->setPos(newx, player->y());
        }
        newx += minWidthByColumn[col] + extraWidthPerColumn;
    }
}

// ---------- Hover Handling ----------

void GameScene::updateHover(const QPointF &scenePos)
{
    auto itemList = items(scenePos, Qt::IntersectsItemBoundingRect, Qt::DescendingOrder, getViewTransform());

    CardZone *zone = findTopmostZone(itemList);
    CardItem *topCard = zone ? findTopmostCardInZone(itemList, zone) : nullptr;
    updateHoveredCard(topCard);
}

void GameScene::updateHoveredCard(CardItem *newCard)
{
    if (hoveredCard && (newCard != hoveredCard))
        hoveredCard->setHovered(false);
    if (newCard && (newCard != hoveredCard))
        newCard->setHovered(true);
    hoveredCard = newCard;
}

CardZone *GameScene::findTopmostZone(const QList<QGraphicsItem *> &items)
{
    for (QGraphicsItem *item : items)
        if (auto *zone = qgraphicsitem_cast<CardZone *>(item))
            return zone;
    return nullptr;
}

CardItem *GameScene::findTopmostCardInZone(const QList<QGraphicsItem *> &items, CardZone *zone)
{
    CardItem *maxZCard = nullptr;
    qreal maxZ = -1;

    for (QGraphicsItem *item : items) {
        CardItem *card = qgraphicsitem_cast<CardItem *>(item);
        if (!card)
            continue;

        if (card->getAttachedTo()) {
            if (card->getAttachedTo()->getZone() != zone->getLogic())
                continue;
        } else if (card->getZone() != zone->getLogic())
            continue;

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
void GameScene::toggleZoneView(Player *player, const QString &zoneName, int numberCards, bool isReversed)
{
    for (auto &view : zoneViews) {
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

    if (zoneName == "grave")
        item->setPos(360, 100);
    else if (zoneName == "rfg")
        item->setPos(380, 120);
    else
        item->setPos(340, 80);
}

/**
 * @brief Adds a revealed zone view (for shown cards).
 * @param player Owning player.
 * @param zone Zone logic.
 * @param cardList List of cards to show.
 * @param withWritePermission Whether edits are allowed.
 */
void GameScene::addRevealedZoneView(Player *player,
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
    while (!zoneViews.isEmpty())
        zoneViews.first()->close();
}

/**
 * @brief Closes the most recently added zone view.
 */
void GameScene::closeMostRecentZoneView()
{
    if (!zoneViews.isEmpty())
        zoneViews.last()->close();
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
    if (event->type() == QEvent::GraphicsSceneMouseMove)
        updateHover(static_cast<QGraphicsSceneMouseEvent *>(event)->scenePos());

    return QGraphicsScene::event(event);
}

void GameScene::timerEvent(QTimerEvent * /*event*/)
{
    QMutableSetIterator<CardItem *> i(cardsToAnimate);
    while (i.hasNext()) {
        i.next();
        if (!i.value()->animationEvent())
            i.remove();
    }
    if (cardsToAnimate.isEmpty())
        animationTimer->stop();
}

void GameScene::registerAnimationItem(AbstractCardItem *card)
{
    cardsToAnimate.insert(static_cast<CardItem *>(card));
    if (!animationTimer->isActive())
        animationTimer->start(10, this);
}

void GameScene::unregisterAnimationItem(AbstractCardItem *card)
{
    cardsToAnimate.remove(static_cast<CardItem *>(card));
    if (cardsToAnimate.isEmpty())
        animationTimer->stop();
}

// ---------- Rubber Band ----------

void GameScene::startRubberBand(const QPointF &selectionOrigin)
{
    emit sigStartRubberBand(selectionOrigin);
}

void GameScene::resizeRubberBand(const QPointF &cursorPoint)
{
    emit sigResizeRubberBand(cursorPoint);
}

void GameScene::stopRubberBand()
{
    emit sigStopRubberBand();
}
