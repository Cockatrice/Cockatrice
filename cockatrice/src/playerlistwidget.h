#ifndef PLAYERLISTWIDGET_H
#define PLAYERLISTWIDGET_H

#include <QTreeWidget>
#include <QMap>
#include <QIcon>

class ServerInfo_PlayerProperties;

class PlayerListWidget : public QTreeWidget {
	Q_OBJECT
private:
	QMap<int, QTreeWidgetItem *> players;
	QIcon readyIcon, notReadyIcon, concededIcon, playerIcon, spectatorIcon;
	bool gameStarted;
public:
	PlayerListWidget(QWidget *parent = 0);
	void retranslateUi();
	void addPlayer(ServerInfo_PlayerProperties *player);
	void removePlayer(int playerId);
	void setActivePlayer(int playerId);
	void updatePing(int playerId, int pingTime);
	void updatePlayerProperties(ServerInfo_PlayerProperties *prop);
	void setGameStarted(bool _gameStarted);
};

#endif
