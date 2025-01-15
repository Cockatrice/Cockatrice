#ifndef COCKATRICE_USER_LIST_MANAGER_H
#define COCKATRICE_USER_LIST_MANAGER_H

#include "../chat_view/user_list_proxy.h"
#include "pb/serverinfo_user.pb.h"

#include <QMap>
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

class UserListManager : public QWidget, public UserlistProxy
{
    Q_OBJECT

private:
    AbstractClient *client;
    ServerInfo_User *ownUserInfo;
    QMap<QString, ServerInfo_User> onlineUsers, buddyUsers, ignoredUsers;

private slots:
    void setOwnUserInfo(const ServerInfo_User &userInfo);
    void populateInitialOnlineUsers();
    void processListUsersResponse(const Response &response);
    void processUserJoinedEvent(const Event_UserJoined &event);
    void processUserLeftEvent(const Event_UserLeft &event);
    void buddyListReceived(const QList<ServerInfo_User> &_buddyList);
    void ignoreListReceived(const QList<ServerInfo_User> &_ignoreList);
    void processAddToListEvent(const Event_AddToList &event);
    void processRemoveFromListEvent(const Event_RemoveFromList &event);

public:
    explicit UserListManager(AbstractClient *_client, QWidget *parent = nullptr);
    ~UserListManager() override;

    [[nodiscard]] QMap<QString, ServerInfo_User> getAllUsersList() const
    {
        return onlineUsers;
    }
    [[nodiscard]] QMap<QString, ServerInfo_User> getBuddyList() const
    {
        return buddyUsers;
    }
    [[nodiscard]] QMap<QString, ServerInfo_User> getIgnoreList() const
    {
        return ignoredUsers;
    }

    bool isOwnUserRegistered() const override;
    QString getOwnUsername() const override;
    bool isUserBuddy(const QString &userName) const override;
    bool isUserIgnored(const QString &userName) const override;
    const ServerInfo_User *getOnlineUser(const QString &userName) const override;

public slots:
    void handleConnect();
    void handleDisconnect();

signals:
    void userLeft(const QString &userName);
    void userJoined(const ServerInfo_User &userInfo);
};

#endif // COCKATRICE_USER_LIST_MANAGER_H
