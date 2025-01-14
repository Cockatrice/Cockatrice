#ifndef COCKATRICE_USER_LIST_MANAGER_H
#define COCKATRICE_USER_LIST_MANAGER_H

#include "pb/serverinfo_user.pb.h"

#include <QWidget>

class AbstractClient;
class Event_AddToList;
class Event_ListRooms;
class Event_RemoveFromList;
class Event_UserJoined;
class Event_UserLeft;
class Response;
class ServerInfo_User;
class TabSupervisor;
class UserList;

class UserListManager : public QWidget
{
    Q_OBJECT

private:
    AbstractClient *client;
    TabSupervisor *tabSupervisor;
    UserList *allUsersList, *buddyList, *ignoreList;

private slots:
    void processUserJoinedEvent(const Event_UserJoined &event);
    void processUserLeftEvent(const Event_UserLeft &event);
    void buddyListReceived(const QList<ServerInfo_User> &_buddyList);
    void ignoreListReceived(const QList<ServerInfo_User> &_ignoreList);
    void processAddToListEvent(const Event_AddToList &event);
    void processRemoveFromListEvent(const Event_RemoveFromList &event);

public:
    explicit UserListManager(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    [[nodiscard]] UserList *getAllUsersList() const
    {
        return allUsersList;
    }
    [[nodiscard]] UserList *getBuddyList() const
    {
        return buddyList;
    }
    [[nodiscard]] UserList *getIgnoreList() const
    {
        return ignoreList;
    }

    void retranslateUi() {};

public slots:
    void handleConnect();
    void handleDisconnect();
    void processListUsersResponse(const Response &response);
    void addToList(const QString &listName, const QString &userName);

signals:
    void userLeft(const QString &userName);
    void userJoined(const ServerInfo_User &userInfo);
};

#endif // COCKATRICE_USER_LIST_MANAGER_H
