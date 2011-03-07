#include "gamescene.h"
#include "player.h"
#include "zoneviewwidget.h"
#include "zoneviewzone.h"
#include "phasestoolbar.h"
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QSet>
#include <QBasicTimer>

GameScene::GameScene(PhasesToolbar *_phasesToolbar, QObject *parent)
	: QGraphicsScene(parent), phasesToolbar(_phasesToolbar)
{
	animationTimer = new QBasicTimer;
	addItem(phasesToolbar);
}

GameScene::~GameScene()
{
	delete animationTimer;
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
	connect(player, SIGNAL(gameConceded()), this, SLOT(rearrange()));
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
		static void processPlayer(Player *p, qreal &w, QPointF &b, bool singlePlayer)
		{
			if (p->getConceded())
				return;
			
			const QRectF br = p->boundingRect();
			if (br.width() > w)
				w = br.width();
			p->setPos(b);
			p->setMirrored((b.y() < playerAreaSpacing) && !singlePlayer);
			b += QPointF(0, br.height() + playerAreaSpacing);
		}
	};
	
	qreal sceneHeight = -playerAreaSpacing;
	for (int i = 0; i < players.size(); ++i)
		if (!players[i]->getConceded())
			sceneHeight += players[i]->boundingRect().height() + playerAreaSpacing;
	phasesToolbar->setHeight(sceneHeight);
	qreal phasesWidth = phasesToolbar->getWidth();
	
	QPointF base(phasesWidth, 0);
	qreal sceneWidth;
	QList<Player *> localPlayers;

	for (int i = 0; i < players.size(); ++i)
		if (!players[i]->getLocal())
			PlayerProcessor::processPlayer(players[i], sceneWidth, base, players.size() == 1);
		else
			localPlayers.append(players[i]);
	
	for (int i = 0; i < localPlayers.size(); ++i)
		PlayerProcessor::processPlayer(localPlayers[i], sceneWidth, base, players.size() == 1);

	sceneWidth += phasesWidth;
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
	minWidth += phasesToolbar->getWidth();
	
	qreal minRatio = minWidth / sceneRect().height();
	if (minRatio > newRatio) {
		// Aspect ratio is dominated by table width.
		setSceneRect(0, 0, minWidth, sceneRect().height());
	} else {
		// Aspect ratio is dominated by window dimensions.
		setSceneRect(0, 0, newRatio * sceneRect().height(), sceneRect().height());
	}
	
	for (int i = 0; i < players.size(); ++i)
		players[i]->processSceneSizeChange(sceneRect().size() - QSizeF(phasesToolbar->getWidth(), 0));
}

void GameScene::updateHover(const QPointF &scenePos)
{
	QList<QGraphicsItem *> itemList = items(scenePos);
	
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
		animationTimer->start(50, this);
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
