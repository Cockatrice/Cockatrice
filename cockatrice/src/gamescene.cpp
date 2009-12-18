#include "gamescene.h"
#include "player.h"
#include "zoneviewlayout.h"

GameScene::GameScene(ZoneViewLayout *_zvLayout, QObject *parent)
	: QGraphicsScene(parent), zvLayout(_zvLayout)
{
	connect(zvLayout, SIGNAL(sizeChanged()), this, SLOT(rearrange()));
	addItem(zvLayout);
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
		static void processPlayer(Player *p, qreal &w, qreal &h, QPointF &b)
		{
			const QRectF br = p->boundingRect();
			if (br.width() > w)
				w = br.width();
			if (h)
				h += playerAreaSpacing;
			h += br.height();
			p->setPos(b);
			b += QPointF(0, br.height() + playerAreaSpacing);
		}
	};

	QPointF base;
	qreal sceneWidth = 0;
	qreal sceneHeight = 0;
	Player *localPlayer = 0;

	for (int i = 0; i < players.size(); ++i)
		if (!players[i]->getLocal())
			PlayerProcessor::processPlayer(players[i], sceneWidth, sceneHeight, base);
		else
			localPlayer = players[i];
	if (localPlayer)
		PlayerProcessor::processPlayer(localPlayer, sceneWidth, sceneHeight, base);

	playersRect = QRectF(0, 0, sceneWidth, sceneHeight);
	
	zvLayout->setPos(QPointF(sceneWidth, 0));
	sceneWidth += zvLayout->size().width();
	if (zvLayout->size().height() > sceneHeight)
		sceneHeight = zvLayout->size().height();
	setSceneRect(sceneRect().x(), sceneRect().y(), sceneWidth, sceneHeight);

	qDebug(QString("rearrange(): w=%1 h=%2").arg(sceneWidth).arg(sceneHeight).toLatin1());
}
