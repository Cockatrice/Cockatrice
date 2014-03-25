#ifndef TAB_ROOM_H
#define TAB_ROOM_H

#include "tab.h"
#include <QGroupBox>
#include <QMap>

namespace google { namespace protobuf { class Message; } }
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
class GameSelector;
class Response;
class PendingCommand;
class ServerInfo_User;

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
    
    QMenu *roomMenu;
    QAction *aLeaveRoom;
    QAction *aIgnoreUnregisteredUsers;
    QString sanitizeHtml(QString dirty) const;
signals:
    void roomClosing(TabRoom *tab);
    void openMessageDialog(const QString &userName, bool focus);
private slots:
    void sendMessage();
    void sayFinished(const Response &response);
    void actLeaveRoom();
    void actIgnoreUnregisteredUsers();
    void ignoreUnregisteredUsersChanged();
    
    void processListGamesEvent(const Event_ListGames &event);
    void processJoinRoomEvent(const Event_JoinRoom &event);
    void processLeaveRoomEvent(const Event_LeaveRoom &event);
    void processRoomSayEvent(const Event_RoomSay &event);
public:
    TabRoom(TabSupervisor *_tabSupervisor, AbstractClient *_client, ServerInfo_User *_ownUser, const ServerInfo_Room &info);
    ~TabRoom();
    void retranslateUi();
    void closeRequest();
    void processRoomEvent(const RoomEvent &event);
    int getRoomId() const { return roomId; }
    const QMap<int, QString> &getGameTypes() const { return gameTypes; }
    QString getChannelName() const { return roomName; }
    QString getTabText() const { return roomName; }
    const ServerInfo_User *getUserInfo() const { return ownUser; }

    PendingCommand *prepareRoomCommand(const ::google::protobuf::Message &cmd);
    void sendRoomCommand(PendingCommand *pend);
};

#endif
