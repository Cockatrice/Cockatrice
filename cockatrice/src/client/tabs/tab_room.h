#ifndef TAB_ROOM_H
#define TAB_ROOM_H

#include "../ui/line_edit_completer.h"
#include "tab.h"

#include <QFocusEvent>
#include <QGroupBox>
#include <QKeyEvent>
#include <QMap>

class UserListProxy;
class UserListManager;
namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google
class AbstractClient;
class UserListWidget;
class QLabel;
class ChatView;
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
class Event_RemoveMessages;
class GameSelector;
class Response;
class PendingCommand;
class ServerInfo_User;
class LineEditCompleter;

class TabRoom : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    int roomId;
    QString roomName;
    ServerInfo_User *ownUser;
    QMap<int, QString> gameTypes;

    GameSelector *gameSelector;
    UserListWidget *userList;
    const UserListProxy *userListProxy;
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
    void notIdle();
private slots:
    void sendMessage();
    void sayFinished(const Response &response);
    void actClearChat();
    void actOpenChatSettings();
    void addMentionTag(QString mentionTag);
    void focusTab();
    void actShowMentionPopup(const QString &sender);
    void actShowPopup(const QString &message);
    void actCompleterChanged();

    void processListGamesEvent(const Event_ListGames &event);
    void processJoinRoomEvent(const Event_JoinRoom &event);
    void processLeaveRoomEvent(const Event_LeaveRoom &event);
    void processRoomSayEvent(const Event_RoomSay &event);
    void processRemoveMessagesEvent(const Event_RemoveMessages &event);
    void refreshShortcuts();

protected slots:
    void closeEvent(QCloseEvent *event) override;

public:
    TabRoom(TabSupervisor *_tabSupervisor,
            AbstractClient *_client,
            ServerInfo_User *_ownUser,
            const ServerInfo_Room &info);
    void retranslateUi() override;
    void tabActivated() override;
    void processRoomEvent(const RoomEvent &event);
    int getRoomId() const
    {
        return roomId;
    }
    const QMap<int, QString> &getGameTypes() const
    {
        return gameTypes;
    }
    QString getChannelName() const
    {
        return roomName;
    }
    QString getTabText() const override
    {
        return roomName;
    }
    const ServerInfo_User *getUserInfo() const
    {
        return ownUser;
    }

    PendingCommand *prepareRoomCommand(const ::google::protobuf::Message &cmd);
    void sendRoomCommand(PendingCommand *pend);
};

#endif
