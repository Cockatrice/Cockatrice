#ifndef TAB_ROOM_H
#define TAB_ROOM_H

#include "tab.h"
#include "protocol_datastructures.h"
#include <QGroupBox>

class AbstractClient;
class UserList;
class QLabel;
class ChatView;
class QLineEdit;
class QTreeView;
class QPushButton;
class QTextTable;
class QCheckBox;
class GamesModel;
class GamesProxyModel;
class RoomEvent;
class ServerInfo_Room;
class ServerInfo_Game;
class Event_ListGames;
class Event_JoinRoom;
class Event_LeaveRoom;
class Event_RoomSay;
class ProtocolResponse;
class TabRoom;

class GameSelector : public QGroupBox {
	Q_OBJECT
private slots:
	void showFullGamesChanged(int state);
	void actCreate();
	void actJoin();
	void checkResponse(ResponseCode response);
signals:
	void gameJoined(int gameId);
private:
	AbstractClient *client;
	TabRoom *room;

	QTreeView *gameListView;
	GamesModel *gameListModel;
	GamesProxyModel *gameListProxyModel;
	QPushButton *createButton, *joinButton, *spectateButton;
	QCheckBox *showFullGamesCheckBox;
public:
	GameSelector(AbstractClient *_client, TabRoom *_room, QWidget *parent = 0);
	void retranslateUi();
	void processGameInfo(ServerInfo_Game *info);
};

class TabRoom : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	int roomId;
	QString roomName;
	QString ownName;
	QMap<int, QString> gameTypes;
	
	GameSelector *gameSelector;
	UserList *userList;
	ChatView *chatView;
	QLabel *sayLabel;
	QLineEdit *sayEdit;
	QGroupBox *chatGroupBox;

	QAction *aLeaveRoom;
	QString sanitizeHtml(QString dirty) const;
signals:
	void roomClosing(TabRoom *tab);
	void openMessageDialog(const QString &userName, bool focus);
private slots:
	void sendMessage();
	void actLeaveRoom();
	void sayFinished(ProtocolResponse *response);
	
	void processListGamesEvent(Event_ListGames *event);
	void processJoinRoomEvent(Event_JoinRoom *event);
	void processLeaveRoomEvent(Event_LeaveRoom *event);
	void processSayEvent(Event_RoomSay *event);
public:
	TabRoom(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &_ownName, ServerInfo_Room *info);
	~TabRoom();
	void retranslateUi();
	void processRoomEvent(RoomEvent *event);
	int getRoomId() const { return roomId; }
	const QMap<int, QString> &getGameTypes() const { return gameTypes; }
	QString getChannelName() const { return roomName; }
	QString getTabText() const { return roomName; }
};

#endif
