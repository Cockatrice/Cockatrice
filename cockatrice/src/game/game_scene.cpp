#include "game_scene.h"

#include "../settings/cache_settings.h"
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
#include <numeric>

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
    for (const auto &zoneView : zoneViews) {
        zoneView->deleteLater();
    }
}

void GameScene::retranslateUi()
{
    for (ZoneViewWidget *view : zoneViews)
        view->retranslateUi();
}

void GameScene::addPlayer(Player *player)
{
    qCInfo(GameScenePlayerAdditionRemovalLog) << "GameScene::addPlayer name=" << player->getPlayerInfo()->getName();

    players << player->getGraphicsItem();
    addItem(player->getGraphicsItem());
    connect(player->getGraphicsItem(), &PlayerGraphicsItem::sizeChanged, this, &GameScene::rearrange);
}

void GameScene::removePlayer(Player *player)
{
    qCInfo(GameScenePlayerAdditionRemovalLog)
        << "GameScene::removePlayer name=" << player->getPlayerInfo()->getName();

    for (ZoneViewWidget *zone : zoneViews) {
        if (zone->getPlayer() == player) {
            zone->close();
        }
    }
    players.removeOne(player->getGraphicsItem());
    removeItem(player->getGraphicsItem());
    rearrange();
}

void GameScene::adjustPlayerRotation(int rotationAdjustment)
{
    playerRotation += rotationAdjustment;
    rearrange();
}

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

// ---------- Zone Views ----------

void GameScene::toggleZoneView(Player *player, const QString &zoneName, int numberCards, bool isReversed)
{
    for (auto &view : zoneViews) {
        ZoneViewZone *temp = view->getZone();
        if (temp->getLogic()->getName() == zoneName &&
            temp->getLogic()->getPlayer() == player &&
            qobject_cast<ZoneViewZoneLogic *>(temp->getLogic())->getNumberCards() == numberCards) {
            view->close();
        }
    }

    ZoneViewWidget *item =
        new ZoneViewWidget(
        player,
        player->getZones().value(zoneName),
        numberCards,
        false,
        false, {}, isReversed);

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

void GameScene::removeZoneView(ZoneViewWidget *item)
{
    zoneViews.removeOne(item);
    removeItem(item);
}

void GameScene::clearViews()
{
    while (!zoneViews.isEmpty())
        zoneViews.first()->close();
}

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

// ---------- View Size ----------

void GameScene::processViewSizeChange(const QSize &newSize)
{
    viewSize = newSize;

    QList<qreal> minWidthByColumn = calculateMinWidthByColumn();
    qreal minWidth = std::accumulate(minWidthByColumn.begin(),
                                     minWidthByColumn.end(), phasesToolbar->getWidth());

    qreal newWidth = calculateNewSceneWidth(newSize, minWidth);
    setSceneRect(0, 0, newWidth, sceneRect().height());

    resizeColumnsAndPlayers(minWidthByColumn, newWidth);
}

// ---------- Hover Handling ----------

void GameScene::updateHover(const QPointF &scenePos)
{
    auto itemList = items(scenePos, Qt::IntersectsItemBoundingRect, Qt::DescendingOrder, getViewTransform());

    CardZone *zone = findTopmostZone(itemList);
    CardItem *topCard = zone ? findTopmostCardInZone(itemList, zone) : nullptr;
    updateHoveredCard(topCard);
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

// ==================================================================
//                      Private Helpers
// ==================================================================

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

QList<Player *> GameScene::rotatePlayers(const QList<Player *> &players, int firstPlayerIndex) const
{
    QList<Player *> rotated = players;
    if (!rotated.isEmpty()) {
        int totalRotation = firstPlayerIndex + playerRotation;
        while (totalRotation < 0)
            totalRotation += rotated.size();
        for (int i = 0; i < totalRotation; ++i)
            rotated.append(rotated.takeFirst());
    }
    return rotated;
}

int GameScene::determineColumnCount(int playerCount) const
{
    return playerCount < SettingsCache::instance().getMinPlayersForMultiColumnLayout() ? 1 : 2;
}

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
        int rowsInColumn = rows - (playersPlaying.size() % columns) * col; // for 2 cols

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

    // Position players
    qreal x = phasesWidth;
    for (int col = 0; col < columns; ++col) {
        qreal y = 0;
        for (int row = 0; row < playersByColumn[col].size(); ++row) {
            PlayerGraphicsItem *player = playersByColumn[col][row];
            player->setPos(x, y);
            player->setMirrored(row != rows - 1);
            y += player->boundingRect().height() + playerAreaSpacing;
        }
        x += columnWidth[col] + playerAreaSpacing;
    }

    return QSizeF(sceneWidth, sceneHeight);
}

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

qreal GameScene::calculateNewSceneWidth(const QSize &newSize, qreal minWidth) const
{
    qreal newRatio = (qreal)newSize.width() / newSize.height();
    qreal minRatio = minWidth / sceneRect().height();

    if (minRatio > newRatio) {
        return minWidth; // dominated by table width
    } else {
        return newRatio * sceneRect().height(); // dominated by window dimensions
    }
}

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

CardZone *GameScene::findTopmostZone(const QList<QGraphicsItem *> &items) const
{
    for (QGraphicsItem *item : items)
        if (auto *zone = qgraphicsitem_cast<CardZone *>(item))
            return zone;
    return nullptr;
}

CardItem *GameScene::findTopmostCardInZone(const QList<QGraphicsItem *> &items, CardZone *zone) const
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

void GameScene::updateHoveredCard(CardItem *newCard)
{
    if (hoveredCard && (newCard != hoveredCard))
        hoveredCard->setHovered(false);
    if (newCard && (newCard != hoveredCard))
        newCard->setHovered(true);
    hoveredCard = newCard;
}
