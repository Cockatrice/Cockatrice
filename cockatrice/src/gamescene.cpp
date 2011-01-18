#include "gamescene.h"
#include "player.h"
#include "zoneviewwidget.h"
#include "zoneviewzone.h"
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QSet>

GameScene::GameScene(QObject *parent)
	: QGraphicsScene(parent)
{
}

void GameScene::retranslateUi()
{
	for (int i = 0; i < views.size(); ++i)
		views[i]->retranslateUi();
}

void GameScene::addPlayer(Player *player)
{
	qDebug("GameScene::addPlayer");
	players << player;
	addItem(player);
	rearrange();
	connect(player, SIGNAL(sizeChanged()), this, SLOT(rearrange()));
}

void GameScene::removePlayer(Player *player)
{
	qDebug("GameScene::removePlayer");
	players.removeAt(players.indexOf(player));
	removeItem(player);
	rearrange();
}

void GameScene::rearrange()
{
	struct PlayerProcessor {
		static void processPlayer(Player *p, qreal &w, qreal &h, QPointF &b, bool singlePlayer)
		{
			const QRectF br = p->boundingRect();
			if (br.width() > w)
				w = br.width();
			if (h)
				h += playerAreaSpacing;
			h += br.height();
			p->setPos(b);
			p->setMirrored((b.y() < playerAreaSpacing) && !singlePlayer);
			b += QPointF(0, br.height() + playerAreaSpacing);
		}
	};

	QPointF base;
	qreal sceneWidth = 0;
	qreal sceneHeight = 0;
	QList<Player *> localPlayers;

	for (int i = 0; i < players.size(); ++i)
		if (!players[i]->getLocal())
			PlayerProcessor::processPlayer(players[i], sceneWidth, sceneHeight, base, players.size() == 1);
		else
			localPlayers.append(players[i]);
	
	for (int i = 0; i < localPlayers.size(); ++i)
		PlayerProcessor::processPlayer(localPlayers[i], sceneWidth, sceneHeight, base, players.size() == 1);

	playersRect = QRectF(0, 0, sceneWidth, sceneHeight);
	
	setSceneRect(sceneRect().x(), sceneRect().y(), sceneWidth, sceneHeight);
	processViewSizeChange(viewSize);
}

void GameScene::toggleZoneView(Player *player, const QString &zoneName, int numberCards)
{
        for (int i = 0; i < views.size(); i++) {
                ZoneViewZone *temp = views[i]->getZone();
                if ((temp->getName() == zoneName) && (temp->getPlayer() == player)) { // view is already open
                        views[i]->close();
                        if (temp->getNumberCards() == numberCards)
                                return;
                }
        }

	ZoneViewWidget *item = new ZoneViewWidget(player, player->getZones().value(zoneName), numberCards, false);
        views.append(item);
        connect(item, SIGNAL(closePressed(ZoneViewWidget *)), this, SLOT(removeZoneView(ZoneViewWidget *)));
	addItem(item);
	item->setPos(100, 100);
}

void GameScene::addRevealedZoneView(Player *player, CardZone *zone, const QList<ServerInfo_Card *> &cardList)
{
	ZoneViewWidget *item = new ZoneViewWidget(player, zone, -2, true, cardList);
	views.append(item);
        connect(item, SIGNAL(closePressed(ZoneViewWidget *)), this, SLOT(removeZoneView(ZoneViewWidget *)));
	addItem(item);
	item->setPos(100, 100);
}

void GameScene::removeZoneView(ZoneViewWidget *item)
{
        views.removeAt(views.indexOf(item));
	removeItem(item);
}

void GameScene::clearViews()
{
	for (int i = 0; i < views.size(); ++i)
		views[i]->close();
}

void GameScene::closeMostRecentZoneView()
{
	if (!views.isEmpty())
		views.last()->close();
}

void GameScene::processViewSizeChange(const QSize &newSize)
{
	viewSize = newSize;
	
	qreal newRatio = ((qreal) newSize.width()) / newSize.height();
	qreal minWidth = 0;
	for (int i = 0; i < players.size(); ++i) {
		qreal w = players[i]->getMinimumWidth();
		if (w > minWidth)
			minWidth = w;
	}
	
	qreal minRatio = minWidth / sceneRect().height();
	if (minRatio > newRatio) {
		// Aspect ratio is dominated by table width.
		setSceneRect(0, 0, minWidth, sceneRect().height());
	} else {
		// Aspect ratio is dominated by window dimensions.
		setSceneRect(0, 0, newRatio * sceneRect().height(), sceneRect().height());
	}
	
	for (int i = 0; i < players.size(); ++i)
		players[i]->processSceneSizeChange(sceneRect().size());
}

bool GameScene::event(QEvent *event)
{
	if (event->type() == QEvent::GraphicsSceneMouseMove) {
		QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
		
		QSet<CardItem *> cardsToUnhover;
		
		QList<QGraphicsItem *> oldItemList = items(mouseEvent->lastScenePos());
		for (int i = 0; i < oldItemList.size(); ++i) {
			CardItem *card = qgraphicsitem_cast<CardItem *>(oldItemList[i]);
			if (card)
				cardsToUnhover.insert(card);
		}
		
		QList<QGraphicsItem *> itemList = items(mouseEvent->scenePos());
		
		// Search for the topmost zone and ignore all cards not belonging to that zone.
		CardZone *zone = 0;
		for (int i = 0; i < itemList.size(); ++i)
			if ((zone = qgraphicsitem_cast<CardZone *>(itemList[i])))
				break;
		
		if (zone) {
			CardItem *maxZCard = 0;
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
				cardsToUnhover.insert(card);
			}
			if (maxZCard) {
				cardsToUnhover.remove(maxZCard);
				maxZCard->setHovered(true);
			}
		}
		QSet<CardItem *>::const_iterator i = cardsToUnhover.constBegin();
		while (i != cardsToUnhover.constEnd()) {
			(*i)->setHovered(false);
			++i;
		}
	}
	return QGraphicsScene::event(event);
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
