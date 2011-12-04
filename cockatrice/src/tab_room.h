#ifndef TAB_ROOM_H
#define TAB_ROOM_H

#include "tab.h"
#include "protocol_datastructures.h"
#include <QGroupBox>
#include <QMap>

class AbstractClient;
class UserList;
class QLabel;
class ChatView;
class QLineEdit;
class QPushButton;
class QTextTable;
class RoomEvent;
class ServerInfo_Room;
class ServerInfo_Game;
class Event_ListGames;
class Event_JoinRoom;
class Event_LeaveRoom;
class Event_RoomSay;
class ProtocolResponse;
class GameSelector;

class TabRoom : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	int roomId;
	QString roomName;
	ServerInfo_User *ownUser;
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
	TabRoom(TabSupervisor *_tabSupervisor, AbstractClient *_client, ServerInfo_User *_ownUser, ServerInfo_Room *info);
	~TabRoom();
	void retranslateUi();
	void closeRequest();
	void processRoomEvent(RoomEvent *event);
	int getRoomId() const { return roomId; }
	const QMap<int, QString> &getGameTypes() const { return gameTypes; }
	QString getChannelName() const { return roomName; }
	QString getTabText() const { return roomName; }
	ServerInfo_User *getCurrentUser() { return ownUser; }
};

#endif
