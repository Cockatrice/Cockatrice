#include "gamescene.h"
#include "player.h"
#include "zoneviewwidget.h"
#include "zoneviewzone.h"
#include <QAction>

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

	qDebug(QString("rearrange(): w=%1 h=%2").arg(sceneWidth).arg(sceneHeight).toLatin1());
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

	ZoneViewWidget *item = new ZoneViewWidget(player, player->getZones().value(zoneName), numberCards);
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
		setSceneRect(sceneRect().x(), sceneRect().y(), minWidth, sceneRect().height());
	} else {
		// Aspect ratio is dominated by window dimensions.
		setSceneRect(sceneRect().x(), sceneRect().y(), newRatio * sceneRect().height(), sceneRect().height());
	}
	
	for (int i = 0; i < players.size(); ++i)
		players[i]->processSceneSizeChange(sceneRect().size());
}
