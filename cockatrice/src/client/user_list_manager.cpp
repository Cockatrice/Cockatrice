#include "user_list_manager.h"

#include "../server/pending_command.h"
#include "../server/user/user_info_box.h"
#include "../server/user/user_list.h"
#include "game_logic/abstract_client.h"
#include "pb/event_add_to_list.pb.h"
#include "pb/event_remove_from_list.pb.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include "pb/response_list_users.pb.h"
#include "pb/session_commands.pb.h"
#include "sound_engine.h"
#include "trice_limits.h"

UserListManager::UserListManager(TabSupervisor *_tabSupervisor, AbstractClient *_client)
    : client(_client), tabSupervisor(_tabSupervisor),
      allUsersList(new UserList(tabSupervisor, client, UserList::AllUsersList)),
      buddyList(new UserList(tabSupervisor, client, UserList::BuddyList)),
      ignoreList(new UserList(tabSupervisor, client, UserList::IgnoreList))
{
    connect(client, &AbstractClient::userJoinedEventReceived, this, &UserListManager::processUserJoinedEvent);
    connect(client, &AbstractClient::userLeftEventReceived, this, &UserListManager::processUserLeftEvent);
    connect(client, &AbstractClient::buddyListReceived, this, &UserListManager::buddyListReceived);
    connect(client, &AbstractClient::ignoreListReceived, this, &UserListManager::ignoreListReceived);
    connect(client, &AbstractClient::addToListEventReceived, this, &UserListManager::processAddToListEvent);
    connect(client, &AbstractClient::removeFromListEventReceived, this, &UserListManager::processRemoveFromListEvent);
}

void UserListManager::handleConnect()
{
}

void UserListManager::handleDisconnect()
{
}

void UserListManager::processListUsersResponse(const Response &response)
{
    const auto &resp = response.GetExtension(Response_ListUsers::ext);

    const int userListSize = resp.user_list_size();
    for (int i = 0; i < userListSize; ++i) {
        const ServerInfo_User &info = resp.user_list(i);
        const QString userName = QString::fromStdString(info.name());
        allUsersList->processUserInfo(info, true);
        ignoreList->setUserOnline(userName, true);
        buddyList->setUserOnline(userName, true);
    }

    allUsersList->sortItems();
    ignoreList->sortItems();
    buddyList->sortItems();
}

void UserListManager::processUserJoinedEvent(const Event_UserJoined &event)
{
    const auto &info = event.user_info();
    const auto &userName = QString::fromStdString(info.name());

    allUsersList->processUserInfo(info, true);
    ignoreList->setUserOnline(userName, true);
    buddyList->setUserOnline(userName, true);

    allUsersList->sortItems();
    ignoreList->sortItems();
    buddyList->sortItems();

    if (buddyList->getUsers().keys().contains(userName)) {
        soundEngine->playSound("buddy_join");
    }

    emit userJoined(info);
}

void UserListManager::processUserLeftEvent(const Event_UserLeft &event)
{
    const auto &userName = QString::fromStdString(event.name());

    if (buddyList->getUsers().keys().contains(userName)) {
        soundEngine->playSound("buddy_leave");
    }

    if (allUsersList->deleteUser(userName)) {
        ignoreList->setUserOnline(userName, false);
        buddyList->setUserOnline(userName, false);
        ignoreList->sortItems();
        buddyList->sortItems();

        emit userLeft(userName);
    }
}

void UserListManager::buddyListReceived(const QList<ServerInfo_User> &_buddyList)
{
    for (const auto &buddyUser : _buddyList) {
        buddyList->processUserInfo(buddyUser, false);
    }
    buddyList->sortItems();
}

void UserListManager::ignoreListReceived(const QList<ServerInfo_User> &_ignoreList)
{
    for (const auto &ignoredUser : _ignoreList) {
        ignoreList->processUserInfo(ignoredUser, false);
    }
    ignoreList->sortItems();
}

void UserListManager::processAddToListEvent(const Event_AddToList &event)
{
    const auto &info = event.user_info();
    const auto online = allUsersList->getUsers().contains(QString::fromStdString(info.name()));
    const auto &list = QString::fromStdString(event.list_name());

    UserList *userList = nullptr;
    if (list == "buddy") {
        userList = buddyList;
    } else if (list == "ignore") {
        userList = ignoreList;
    }

    if (!userList) {
        return;
    }

    userList->processUserInfo(info, online);
    userList->sortItems();
}

void UserListManager::processRemoveFromListEvent(const Event_RemoveFromList &event)
{
    const auto &list = QString::fromStdString(event.list_name());
    const auto &user = QString::fromStdString(event.user_name());

    UserList *userList = nullptr;
    if (list == "buddy") {
        userList = buddyList;
    } else if (list == "ignore") {
        userList = ignoreList;
    }

    if (!userList) {
        return;
    }

    userList->deleteUser(user);
}

void UserListManager::addToList(const QString &listName, const QString &userName)
{
    Command_AddToList cmd;
    cmd.set_list(listName.toStdString());
    cmd.set_user_name(userName.toStdString());

    client->sendCommand(client->prepareSessionCommand(cmd));
}