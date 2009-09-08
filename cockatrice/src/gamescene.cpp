#include "gamescene.h"
#include "player.h"
#include "zoneviewlayout.h"

GameScene::GameScene(ZoneViewLayout *_zvLayout, QObject *parent)
	: QGraphicsScene(parent), zvLayout(_zvLayout)
{
	connect(zvLayout, SIGNAL(sizeChanged()), this, SLOT(updateSceneSize()));
	addItem(zvLayout);
}

void GameScene::updateSceneSize()
{
	int sceneWidth = 0;
	int sceneHeight = 0;
	for (int i = 0; i < players.size(); ++i) {
		const QRectF br = players[i]->boundingRect();
		if (i)
			sceneHeight += playerAreaSpacing;
		sceneHeight += br.height();
		if (br.width() > sceneWidth)
			sceneWidth = br.width();
	}
	sceneWidth += zvLayout->size().width();
	qDebug(QString("updateSceneSize: w=%1 h=%2").arg(sceneWidth).arg(sceneHeight).toLatin1());
	setSceneRect(sceneRect().x(), sceneRect().y(), sceneWidth, sceneHeight);
}

void GameScene::addPlayer(Player *player)
{
	players << player;
	updateSceneSize();
	addItem(player);
	rearrangePlayers();
}

void GameScene::removePlayer(Player *player)
{
	players.removeAt(players.indexOf(player));
	removeItem(player);
	updateSceneSize();
	rearrangePlayers();
}

void GameScene::rearrangePlayers()
{
	QPointF base;
	qreal maxWidth = 0;
	Player *localPlayer = 0;
	for (int i = 0; i < players.size(); ++i) {
		QRectF br = players[i]->boundingRect();
		if (br.width() > maxWidth)
			maxWidth = br.width();
		if (!players[i]->getLocal()) {
			players[i]->setPos(base);
			// Change this for better multiplayer support.
			base += QPointF(0, br.height() + playerAreaSpacing);
		} else
			localPlayer = players[i];
	}
	if (localPlayer)
		localPlayer->setPos(base);
	zvLayout->setPos(QPointF(maxWidth, 0));
}
