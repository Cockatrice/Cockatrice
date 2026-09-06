/**
 * @file tab_server.h
 * @ingroup ServerTabs
 */
//! \todo Document this file.

#ifndef TAB_SERVER_H
#define TAB_SERVER_H

#include "tab.h"

#include <QGroupBox>
#include <QMap>
#include <QTextBrowser>
#include <QTreeWidget>

class AbstractClient;
class QTextEdit;
class QLabel;
class UserListWidget;
class QPushButton;

class Event_ListRooms;
class Event_ServerMessage;
class Response;
class ServerInfo_Room;
class CommandContainer;

class RoomSelector : public QGroupBox
{
    Q_OBJECT
private:
    QTreeWidget *roomList;
    QPushButton *joinButton;
    AbstractClient *client;
    QString getRoomPermissionDisplay(const ServerInfo_Room &room);
private slots:
    void processListRoomsEvent(const Event_ListRooms &event);
    void joinClicked();
signals:
    void joinRoomRequest(int, bool setCurrent);

public:
    explicit RoomSelector(AbstractClient *_client, QWidget *parent = nullptr);
    void retranslateUi();
};

class TabServer : public Tab
{
    Q_OBJECT
signals:
    void roomJoined(const ServerInfo_Room &info, bool setCurrent);
    void roomJoinFailed(int roomId);
private slots:
    void processServerMessageEvent(const Event_ServerMessage &event);
    void joinRoomFinished(const Response &resp,
                          const CommandContainer &commandContainer,
                          const QVariant &extraData,
                          int roomId);

private:
    void leaveAndRejoinRoom(int roomId, bool setCurrent);

    AbstractClient *client;
    RoomSelector *roomSelector;
    QTextBrowser *serverInfoBox;
    bool shouldEmitUpdate = false;
    /** Room ids with a join command in flight, mapped to whether the tab should be focused once it opens. */
    QMap<int, bool> pendingRoomJoins;

public:
    TabServer(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    void joinRoom(int id, bool setCurrent);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Server");
    }
};

#endif
