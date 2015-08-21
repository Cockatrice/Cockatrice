#ifndef TAB_ROOM_H
#define TAB_ROOM_H

#include "tab.h"
#include "lineeditcompleter.h"
#include <QGroupBox>
#include <QMap>
#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>

namespace google { namespace protobuf { class Message; } }
class AbstractClient;
class UserList;
class QLabel;
class ChatView;
class QLineEdit;
class QPushButton;
class QTextTable;
class QCompleter;
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
class LineEditCompleter;

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
    LineEditCompleter *sayEdit;
    QGroupBox *chatGroupBox;
    
    QMenu *roomMenu;
    QAction *aLeaveRoom;
    QAction *aOpenChatSettings;
    QAction *aClearChat;
    QString sanitizeHtml(QString dirty) const;

    QStringList autocompleteUserList;
    QCompleter *completer;
signals:
    void roomClosing(TabRoom *tab);
    void openMessageDialog(const QString &userName, bool focus);
    void maximizeClient();
private slots:
    void sendMessage();
    void sayFinished(const Response &response);
    void actLeaveRoom();
    void actClearChat();
    void actOpenChatSettings();
    void addMentionTag(QString mentionTag);
    void focusTab();
    void actShowMentionPopup(QString &sender);
    void actCompleterChanged();

    void processListGamesEvent(const Event_ListGames &event);
    void processJoinRoomEvent(const Event_JoinRoom &event);
    void processLeaveRoomEvent(const Event_LeaveRoom &event);
    void processRoomSayEvent(const Event_RoomSay &event);
    void refreshShortcuts();
public:
    TabRoom(TabSupervisor *_tabSupervisor, AbstractClient *_client, ServerInfo_User *_ownUser, const ServerInfo_Room &info);
    ~TabRoom();
    void retranslateUi();
    void closeRequest();
    void tabActivated();
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
