#ifndef TAB_ROOM_H
#define TAB_ROOM_H

#include "tab.h"
#include "protocol_datastructures.h"
#include <QGroupBox>
#include <QTextEdit>

class AbstractClient;
class UserList;
class QLabel;
class QTextEdit;
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
	int roomId;

	QTreeView *gameListView;
	GamesModel *gameListModel;
	GamesProxyModel *gameListProxyModel;
	QPushButton *createButton, *joinButton, *spectateButton;
	QCheckBox *showFullGamesCheckBox;
public:
	GameSelector(AbstractClient *_client, int _roomId, QWidget *parent = 0);
	void retranslateUi();
	void processGameInfo(ServerInfo_Game *info);
};

class ChatView : public QTextEdit {
	Q_OBJECT;
private:
	QTextTable *table;
	QString ownName;
public:
	ChatView(const QString &_ownName, QWidget *parent = 0);
	void appendMessage(const QString &sender, const QString &message);
};

class TabRoom : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	int roomId;
	QString roomName;
	QString ownName;
	
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
	
	void processListGamesEvent(Event_ListGames *event);
	void processJoinRoomEvent(Event_JoinRoom *event);
	void processLeaveRoomEvent(Event_LeaveRoom *event);
	void processSayEvent(Event_RoomSay *event);
public:
	TabRoom(AbstractClient *_client, const QString &_ownName, ServerInfo_Room *info);
	~TabRoom();
	void retranslateUi();
	void processRoomEvent(RoomEvent *event);
	int getRoomId() const { return roomId; }
	QString getChannelName() const { return roomName; }
	QString getTabText() const { return roomName; }
};

#endif
