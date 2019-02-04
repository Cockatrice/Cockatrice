#include "gamescene.h"
#include "carditem.h"
#include "phasestoolbar.h"
#include "player.h"
#include "settingscache.h"
#include "zoneviewwidget.h"
#include "zoneviewzone.h"
#include <QAction>
#include <QBasicTimer>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QSet>
#include <math.h>

GameScene::GameScene(PhasesToolbar *_phasesToolbar, QObject *parent)
    : QGraphicsScene(parent), phasesToolbar(_phasesToolbar), viewSize(QSize()), playerRotation(0)
{
    animationTimer = new QBasicTimer;
    addItem(phasesToolbar);
    connect(settingsCache, SIGNAL(minPlayersForMultiColumnLayoutChanged()), this, SLOT(rearrange()));

    rearrange();
}

GameScene::~GameScene()
{
    delete animationTimer;
}

void GameScene::retranslateUi()
{
    for (int i = 0; i < zoneViews.size(); ++i)
        zoneViews[i]->retranslateUi();
}

void GameScene::addPlayer(Player *player)
{
    qDebug() << "GameScene::addPlayer name=" << player->getName();
    players << player;
    addItem(player);
    connect(player, SIGNAL(sizeChanged()), this, SLOT(rearrange()));
    connect(player, SIGNAL(playerCountChanged()), this, SLOT(rearrange()));
}

void GameScene::removePlayer(Player *player)
{
    qDebug() << "GameScene::removePlayer name=" << player->getName();
    players.removeAt(players.indexOf(player));
    removeItem(player);
    rearrange();
}

void GameScene::adjustPlayerRotation(int rotationAdjustment)
{
    playerRotation += rotationAdjustment;
    rearrange();
}

void GameScene::rearrange()
{
    playersByColumn.clear();

    // Create the list of players playing, noting the first player's index.
    QList<Player *> playersPlaying;
    int firstPlayerIndex = 0;
    bool firstPlayerFound = false;
    QListIterator<Player *> playersIter(players);
    while (playersIter.hasNext()) {
        Player *p = playersIter.next();
        if (!p->getConceded()) {
            playersPlaying.append(p);
            if (!firstPlayerFound && (p->getLocal())) {
                firstPlayerIndex = playersPlaying.size() - 1;
                firstPlayerFound = true;
            }
        }
    }

    // Rotate the players playing list so that first player is first, then
    // adjust by the additional rotation setting.
    if (!playersPlaying.isEmpty()) {
        int totalRotation = firstPlayerIndex + playerRotation;
        while (totalRotation < 0)
            totalRotation += playersPlaying.size();
        for (int i = 0; i < totalRotation; ++i) {
            playersPlaying.append(playersPlaying.takeFirst());
        }
    }

    const int playersCount = playersPlaying.size();
    const int columns = playersCount < settingsCache->getMinPlayersForMultiColumnLayout() ? 1 : 2;
    const int rows = ceil((qreal)playersCount / columns);
    qreal sceneHeight = 0, sceneWidth = -playerAreaSpacing;
    QList<int> columnWidth;

    QListIterator<Player *> playersPlayingIter(playersPlaying);
    for (int col = 0; col < columns; ++col) {
        playersByColumn.append(QList<Player *>());
        columnWidth.append(0);
        qreal thisColumnHeight = -playerAreaSpacing;
        const int rowsInColumn = rows - (playersCount % columns) * col; // only correct for max. 2 cols
        for (int j = 0; j < rowsInColumn; ++j) {
            Player *player = playersPlayingIter.next();
            if (col == 0)
                playersByColumn[col].prepend(player);
            else
                playersByColumn[col].append(player);
            thisColumnHeight += player->boundingRect().height() + playerAreaSpacing;
            if (player->boundingRect().width() > columnWidth[col])
                columnWidth[col] = player->boundingRect().width();
        }
        if (thisColumnHeight > sceneHeight)
            sceneHeight = thisColumnHeight;
        sceneWidth += columnWidth[col] + playerAreaSpacing;
    }

    phasesToolbar->setHeight(sceneHeight);
    qreal phasesWidth = phasesToolbar->getWidth();
    sceneWidth += phasesWidth;

    qreal x = phasesWidth;
    for (int col = 0; col < columns; ++col) {
        qreal y = 0;
        for (int row = 0; row < playersByColumn[col].size(); ++row) {
            Player *player = playersByColumn[col][row];
            player->setPos(x, y);
            player->setMirrored(row != rows - 1);
            y += player->boundingRect().height() + playerAreaSpacing;
        }
        x += columnWidth[col] + playerAreaSpacing;
    }

    setSceneRect(sceneRect().x(), sceneRect().y(), sceneWidth, sceneHeight);
    processViewSizeChange(viewSize);
}

void GameScene::toggleZoneView(Player *player, const QString &zoneName, int numberCards)
{
    for (int i = 0; i < zoneViews.size(); i++) {
        ZoneViewZone *temp = zoneViews[i]->getZone();
        if ((temp->getName() == zoneName) && (temp->getPlayer() == player)) { // view is already open
            zoneViews[i]->close();
            if (temp->getNumberCards() == numberCards)
                return;
        }
    }

    ZoneViewWidget *item = new ZoneViewWidget(player, player->getZones().value(zoneName), numberCards, false);
    zoneViews.append(item);
    connect(item, SIGNAL(closePressed(ZoneViewWidget *)), this, SLOT(removeZoneView(ZoneViewWidget *)));
    addItem(item);
    if (zoneName == "grave")
        item->setPos(360, 100);
    else if (zoneName == "rfg")
        item->setPos(380, 120);
    else
        item->setPos(340, 80);
}

void GameScene::addRevealedZoneView(Player *player,
                                    CardZone *zone,
                                    const QList<const ServerInfo_Card *> &cardList,
                                    bool withWritePermission)
{
    ZoneViewWidget *item = new ZoneViewWidget(player, zone, -2, true, withWritePermission, cardList);
    zoneViews.append(item);
    connect(item, SIGNAL(closePressed(ZoneViewWidget *)), this, SLOT(removeZoneView(ZoneViewWidget *)));
    addItem(item);
    item->setPos(600, 80);
}

void GameScene::removeZoneView(ZoneViewWidget *item)
{
    zoneViews.removeAt(zoneViews.indexOf(item));
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

QTransform GameScene::getViewTransform() const
{
    return views().at(0)->transform();
}

QTransform GameScene::getViewportTransform() const
{
    return views().at(0)->viewportTransform();
}

void GameScene::processViewSizeChange(const QSize &newSize)
{
    viewSize = newSize;

    qreal newRatio = ((qreal)newSize.width()) / newSize.height();
    qreal minWidth = 0;
    QList<qreal> minWidthByColumn;
    for (int col = 0; col < playersByColumn.size(); ++col) {
        minWidthByColumn.append(0);
        for (int row = 0; row < playersByColumn[col].size(); ++row) {
            qreal w = playersByColumn[col][row]->getMinimumWidth();
            if (w > minWidthByColumn[col])
                minWidthByColumn[col] = w;
        }
        minWidth += minWidthByColumn[col];
    }
    minWidth += phasesToolbar->getWidth();

    qreal minRatio = minWidth / sceneRect().height();
    qreal newWidth;
    if (minRatio > newRatio) {
        // Aspect ratio is dominated by table width.
        newWidth = minWidth;
    } else {
        // Aspect ratio is dominated by window dimensions.
        newWidth = newRatio * sceneRect().height();
    }
    setSceneRect(0, 0, newWidth, sceneRect().height());

    qreal extraWidthPerColumn = (newWidth - minWidth) / playersByColumn.size();
    qreal newx = phasesToolbar->getWidth();
    for (int col = 0; col < playersByColumn.size(); ++col) {
        for (int row = 0; row < playersByColumn[col].size(); ++row) {
            playersByColumn[col][row]->processSceneSizeChange(minWidthByColumn[col] + extraWidthPerColumn);
            playersByColumn[col][row]->setPos(newx, playersByColumn[col][row]->y());
        }
        newx += minWidthByColumn[col] + extraWidthPerColumn;
    }
}

void GameScene::updateHover(const QPointF &scenePos)
{
    QList<QGraphicsItem *> itemList =
        items(scenePos, Qt::IntersectsItemBoundingRect, Qt::DescendingOrder, getViewTransform());

    // Search for the topmost zone and ignore all cards not belonging to that zone.
    CardZone *zone = 0;
    for (int i = 0; i < itemList.size(); ++i)
        if ((zone = qgraphicsitem_cast<CardZone *>(itemList[i])))
            break;

    CardItem *maxZCard = 0;
    if (zone) {
        qreal maxZ = -1;
        for (int i = 0; i < itemList.size(); ++i) {
            CardItem *card = qgraphicsitem_cast<CardItem *>(itemList[i]);
            if (!card)
                continue;
            if (card->getAttachedTo()) {
                if (card->getAttachedTo()->getZone() != zone)
                    continue;
            } else if (card->getZone() != zone)
                continue;

            if (card->getRealZValue() > maxZ) {
                maxZ = card->getRealZValue();
                maxZCard = card;
            }
        }
    }
    if (hoveredCard && (maxZCard != hoveredCard))
        hoveredCard->setHovered(false);
    if (maxZCard && (maxZCard != hoveredCard))
        maxZCard->setHovered(true);
    hoveredCard = maxZCard;
}

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
