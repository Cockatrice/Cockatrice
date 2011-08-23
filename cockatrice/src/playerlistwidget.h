#ifndef PLAYERLISTWIDGET_H
#define PLAYERLISTWIDGET_H

#include <QTreeWidget>
#include <QMap>
#include <QIcon>
#include <QStyledItemDelegate>

class ServerInfo_PlayerProperties;
class TabSupervisor;
class AbstractClient;
class TabGame;

class PlayerListItemDelegate : public QStyledItemDelegate {
public:
	PlayerListItemDelegate(QObject *const parent);
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class PlayerListTWI : public QTreeWidgetItem {
public:
	PlayerListTWI();
	bool operator<(const QTreeWidgetItem &other) const;
};

class PlayerListWidget : public QTreeWidget {
	Q_OBJECT
private:
	PlayerListItemDelegate *itemDelegate;
	QMap<int, QTreeWidgetItem *> players;
	TabSupervisor *tabSupervisor;
	AbstractClient *client;
	TabGame *game;
	bool gameCreator;
	QIcon readyIcon, notReadyIcon, concededIcon, playerIcon, spectatorIcon;
	bool gameStarted;
signals:
	void openMessageDialog(const QString &userName, bool focus);
public:
	PlayerListWidget(TabSupervisor *_tabSupervisor, AbstractClient *_client, TabGame *_game, bool _gameCreator, QWidget *parent = 0);
	void retranslateUi();
	void addPlayer(ServerInfo_PlayerProperties *player);
	void removePlayer(int playerId);
	void setActivePlayer(int playerId);
	void updatePing(int playerId, int pingTime);
	void updatePlayerProperties(ServerInfo_PlayerProperties *prop);
	void setGameStarted(bool _gameStarted);
	void showContextMenu(const QPoint &pos, const QModelIndex &index);
};

#endif
