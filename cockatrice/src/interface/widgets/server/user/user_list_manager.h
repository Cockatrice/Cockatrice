/**
 * @file user_list_manager.h
 * @ingroup Lobby
 */
//! \todo Document this file.

#ifndef COCKATRICE_USER_LIST_MANAGER_H
#define COCKATRICE_USER_LIST_MANAGER_H

#include "user_list_proxy.h"

#include <QMap>
#include <QWidget>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

class AbstractClient;
class Event_AddToList;
class Event_ListRooms;
class Event_RemoveFromList;
class Event_UserJoined;
class Event_UserLeft;
class Response;
class ServerInfo_User;
class TabSupervisor;

class UserListManager : public QObject, public UserListProxy
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
    explicit UserListManager(AbstractClient *_client, QObject *parent = nullptr);
    ~UserListManager() override;

    [[nodiscard]] const QMap<QString, ServerInfo_User> &getAllUsersList() const
    {
        return onlineUsers;
    }

    [[nodiscard]] const QMap<QString, ServerInfo_User> &getBuddyList() const
    {
        return buddyUsers;
    }

    [[nodiscard]] const QMap<QString, ServerInfo_User> &getIgnoreList() const
    {
        return ignoredUsers;
    }

    [[nodiscard]] bool isOwnUserRegistered() const override;
    [[nodiscard]] QString getOwnUsername() const override;
    [[nodiscard]] bool isUserBuddy(const QString &userName) const override;
    [[nodiscard]] bool isUserIgnored(const QString &userName) const override;
    [[nodiscard]] const ServerInfo_User *getOnlineUser(const QString &userName) const override;

public slots:
    void handleConnect();
    void handleDisconnect();

signals:
    /**
     * The entire list needs to be rebuilt from scratch.
     * Fired on disconnect, reconnect, and initial bulk loads
     * (Command_ListUsers response, initial buddy/ignore lists).
     */
    void listReset();

    // ── Online user presence ──────────────────────────────────────────────────
    /** A user came online (or joined the room). Full ServerInfo_User available. */
    void userJoinedOnline(const ServerInfo_User &user);
    /** A user went offline (or left the room). */
    void userLeftOnline(const QString &userName);

    // ── Buddy list mutations (individual, post-login) ─────────────────────────
    void addedToBuddyList(const ServerInfo_User &user);
    void removedFromBuddyList(const QString &userName);

    // ── Ignore list mutations (individual, post-login) ────────────────────────
    void addedToIgnoreList(const ServerInfo_User &user);
    void removedFromIgnoreList(const QString &userName);
};

#endif // COCKATRICE_USER_LIST_MANAGER_H
