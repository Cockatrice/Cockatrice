#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QList>

class Player;
class ZoneViewLayout;

class GameScene : public QGraphicsScene {
	Q_OBJECT
private:
	static const int playerAreaSpacing = 5;
	
	QList<Player *> players;
	ZoneViewLayout *zvLayout;
	QRectF playersRect;
public:
	GameScene(ZoneViewLayout *_zvLayout, QObject *parent = 0);
	const QRectF &getPlayersRect() const { return playersRect; }
public slots:
	void addPlayer(Player *player);
	void removePlayer(Player *player);
private slots:
	void rearrange();
};

#endif
