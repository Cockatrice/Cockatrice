#ifndef GAMESELECTOR_H
#define GAMESELECTOR_H

#include <QGroupBox>
//#include "tab_room.h"
#include "gametypemap.h"
#include "pb/response.pb.h"

class QTreeView;
class GamesModel;
class GamesProxyModel;
class QPushButton;
class QCheckBox;
class AbstractClient;
class TabSupervisor;
class TabRoom;
class ServerInfo_Game;

class GameSelector : public QGroupBox {
	Q_OBJECT
private slots:
	void showUnavailableGamesChanged(int state);
	void actCreate();
	void actJoin();
	void checkResponse(Response::ResponseCode response);
signals:
	void gameJoined(int gameId);
private:
	AbstractClient *client;
	TabSupervisor *tabSupervisor;
	TabRoom *room;

	QTreeView *gameListView;
	GamesModel *gameListModel;
	GamesProxyModel *gameListProxyModel;
	QPushButton *createButton, *joinButton, *spectateButton;
	QCheckBox *showUnavailableGamesCheckBox;
public:
	GameSelector(AbstractClient *_client, TabSupervisor *_tabSupervisor, TabRoom *_room, const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QWidget *parent = 0);
	void retranslateUi();
	void processGameInfo(const ServerInfo_Game &info);
};

#endif
