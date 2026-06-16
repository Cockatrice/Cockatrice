#include "user_list_manager.h"

#include "../../client/sound_engine.h"
#include "user_info_box.h"

#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/event_add_to_list.pb.h>
#include <libcockatrice/protocol/pb/event_remove_from_list.pb.h>
#include <libcockatrice/protocol/pb/event_user_joined.pb.h>
#include <libcockatrice/protocol/pb/event_user_left.pb.h>
#include <libcockatrice/protocol/pb/response_list_users.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>
#include <libcockatrice/protocol/pending_command.h>

UserListManager::UserListManager(AbstractClient *_client, QObject *parent)
    : QObject(parent), client(_client), ownUserInfo(nullptr)
{
    connect(client, &AbstractClient::userJoinedEventReceived, this, &UserListManager::processUserJoinedEvent);
    connect(client, &AbstractClient::userLeftEventReceived, this, &UserListManager::processUserLeftEvent);
    connect(client, &AbstractClient::buddyListReceived, this, &UserListManager::buddyListReceived);
    connect(client, &AbstractClient::ignoreListReceived, this, &UserListManager::ignoreListReceived);
    connect(client, &AbstractClient::addToListEventReceived, this, &UserListManager::processAddToListEvent);
    connect(client, &AbstractClient::removeFromListEventReceived, this, &UserListManager::processRemoveFromListEvent);
    connect(client, &AbstractClient::userInfoChanged, this, &UserListManager::setOwnUserInfo);
}

UserListManager::~UserListManager()
{
    handleDisconnect();
}

void UserListManager::handleConnect()
{
    populateInitialOnlineUsers();
}

void UserListManager::handleDisconnect()
{
    onlineUsers.clear();
    buddyUsers.clear();
    ignoredUsers.clear();

    delete ownUserInfo;
    ownUserInfo = nullptr;

    // Full rebuild — all lists are gone
    emit listReset();
}

void UserListManager::setOwnUserInfo(const ServerInfo_User &userInfo)
{
    ownUserInfo = new ServerInfo_User(userInfo);
}

void UserListManager::populateInitialOnlineUsers()
{
    PendingCommand *pend = client->prepareSessionCommand(Command_ListUsers());
    connect(pend, &PendingCommand::finished, this, &UserListManager::processListUsersResponse);
    client->sendCommand(pend);
}

void UserListManager::processListUsersResponse(const Response &response)
{
    const auto &resp = response.GetExtension(Response_ListUsers::ext);

    const int userListSize = resp.user_list_size();
    for (int i = 0; i < userListSize; ++i) {
        const ServerInfo_User &info = resp.user_list(i);
        onlineUsers.insert(QString::fromStdString(info.name()), info);
    }

    // Bulk load complete — widgets rebuild once from the now-populated map
    emit listReset();
}

void UserListManager::processUserJoinedEvent(const Event_UserJoined &event)
{
    const auto &info = event.user_info();
    const QString name = QString::fromStdString(info.name());
    onlineUsers.insert(name, info);

    emit userJoinedOnline(info);
}

void UserListManager::processUserLeftEvent(const Event_UserLeft &event)
{
    const QString name = QString::fromStdString(event.name());
    onlineUsers.remove(name);

    emit userLeftOnline(name);
}

void UserListManager::buddyListReceived(const QList<ServerInfo_User> &_buddyList)
{
    for (const auto &user : _buddyList) {
        buddyUsers.insert(QString::fromStdString(user.name()), user);
    }

    // Bulk load — one reset covers all newly added entries
    emit listReset();
}

void UserListManager::ignoreListReceived(const QList<ServerInfo_User> &_ignoreList)
{
    for (const auto &user : _ignoreList) {
        ignoredUsers.insert(QString::fromStdString(user.name()), user);
    }

    // Bulk load — one reset covers all newly added entries
    emit listReset();
}

void UserListManager::processAddToListEvent(const Event_AddToList &event)
{
    const auto &user = event.user_info();
    const QString userName = QString::fromStdString(user.name());
    const QString listType = QString::fromStdString(event.list_name());

    if (listType == "buddy") {
        buddyUsers.insert(userName, user);
        emit addedToBuddyList(user);
    } else if (listType == "ignore") {
        ignoredUsers.insert(userName, user);
        emit addedToIgnoreList(user);
    }
}

void UserListManager::processRemoveFromListEvent(const Event_RemoveFromList &event)
{
    const QString listType = QString::fromStdString(event.list_name());
    const QString userName = QString::fromStdString(event.user_name());

    if (listType == "buddy") {
        buddyUsers.remove(userName);
        emit removedFromBuddyList(userName);
    } else if (listType == "ignore") {
        ignoredUsers.remove(userName);
        emit removedFromIgnoreList(userName);
    }
}

bool UserListManager::isOwnUserRegistered() const
{
    return ownUserInfo != nullptr && (ownUserInfo->user_level() & ServerInfo_User::IsRegistered) != 0;
}

QString UserListManager::getOwnUsername() const
{
    return ownUserInfo != nullptr ? QString::fromStdString(ownUserInfo->name()) : QString();
}

bool UserListManager::isUserBuddy(const QString &userName) const
{
    return buddyUsers.contains(userName);
}

bool UserListManager::isUserIgnored(const QString &userName) const
{
    return ignoredUsers.contains(userName);
}

const ServerInfo_User *UserListManager::getOnlineUser(const QString &userName) const
{
    const QString lower = userName.toLower();
    const auto it = std::find_if(onlineUsers.begin(), onlineUsers.end(), [&lower](const ServerInfo_User &user) {
        return lower == QString::fromStdString(user.name()).toLower();
    });
    return it != onlineUsers.end() ? &*it : nullptr;
}