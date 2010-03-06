#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QList>

class Player;
class ZoneViewWidget;

class GameScene : public QGraphicsScene {
	Q_OBJECT
private:
	static const int playerAreaSpacing = 5;
	
	QList<Player *> players;
	QRectF playersRect;
	QList<ZoneViewWidget *> views;
public:
	GameScene(QObject *parent = 0);
	void retranslateUi();
	const QRectF &getPlayersRect() const { return playersRect; }
public slots:
	void toggleZoneView(Player *player, const QString &zoneName, int numberCards);
	void removeZoneView(ZoneViewWidget *item);
	void addPlayer(Player *player);
	void removePlayer(Player *player);
	void clearViews();
private slots:
	void rearrange();
};

#endif
