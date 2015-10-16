#ifndef TAB_SERVER_H
#define TAB_SERVER_H

#include <QGroupBox>
#include <QTreeWidget>
#include <QTextBrowser>
#include "tab.h"

class AbstractClient;
class QTextEdit;
class QLabel;
class UserList;
class QPushButton;

class Event_ListRooms;
class Event_ServerMessage;
class Response;
class ServerInfo_Room;
class CommandContainer;

class RoomSelector : public QGroupBox {
    Q_OBJECT
private:
    QTreeWidget *roomList;
    QPushButton *joinButton;
    AbstractClient *client;
private slots:
    void processListRoomsEvent(const Event_ListRooms &event);
    void joinClicked();
signals:
    void joinRoomRequest(int, bool setCurrent);
public:
    RoomSelector(AbstractClient *_client, QWidget *parent = 0);
    void retranslateUi();
};

class TabServer : public Tab {
    Q_OBJECT
signals:
    void roomJoined(const ServerInfo_Room &info, bool setCurrent);
private slots:
    void processServerMessageEvent(const Event_ServerMessage &event);
    void joinRoom(int id, bool setCurrent);
    void joinRoomFinished(const Response &resp, const CommandContainer &commandContainer, const QVariant &extraData);
private:
    AbstractClient *client;
    RoomSelector *roomSelector;
    QTextBrowser *serverInfoBox;
public:
    TabServer(TabSupervisor *_tabSupervisor, AbstractClient *_client, QWidget *parent = 0);
    void retranslateUi();
    QString getTabText() const { return tr("Server"); }
};

#endif
