#ifndef PLAYERLISTWIDGET_H
#define PLAYERLISTWIDGET_H

#include <QTreeWidget>
#include <QMap>

class ServerInfo_Player;

class PlayerListWidget : public QTreeWidget {
	Q_OBJECT
private:
	QMap<int, QTreeWidgetItem *> players;
public:
	PlayerListWidget(QWidget *parent = 0);
	void retranslateUi();
	void addPlayer(ServerInfo_Player *player);
	void removePlayer(int playerId);
	void setActivePlayer(int playerId);
};

#endif
