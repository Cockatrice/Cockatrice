#include "tab_userlists.h"
#include "userlist.h"
#include "userinfobox.h"
#include "abstractclient.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "pending_command.h"
#include "pb/session_commands.pb.h"
#include "pb/response_list_users.pb.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include "pb/event_add_to_list.pb.h"
#include "pb/event_remove_from_list.pb.h"

TabUserLists::TabUserLists(TabSupervisor *_tabSupervisor, AbstractClient *_client, const ServerInfo_User &userInfo, QWidget *parent)
    : Tab(_tabSupervisor, parent), client(_client)
{
    allUsersList = new UserList(_tabSupervisor, client, UserList::AllUsersList);
    buddyList = new UserList(_tabSupervisor, client, UserList::BuddyList);
    ignoreList = new UserList(_tabSupervisor, client, UserList::IgnoreList);
    userInfoBox = new UserInfoBox(client, false);
    userInfoBox->updateInfo(userInfo);
    
    connect(allUsersList, SIGNAL(openMessageDialog(const QString &, bool)), this, SIGNAL(openMessageDialog(const QString &, bool)));
    connect(buddyList, SIGNAL(openMessageDialog(const QString &, bool)), this, SIGNAL(openMessageDialog(const QString &, bool)));
    connect(ignoreList, SIGNAL(openMessageDialog(const QString &, bool)), this, SIGNAL(openMessageDialog(const QString &, bool)));
    
    connect(client, SIGNAL(userJoinedEventReceived(const Event_UserJoined &)), this, SLOT(processUserJoinedEvent(const Event_UserJoined &)));
    connect(client, SIGNAL(userLeftEventReceived(const Event_UserLeft &)), this, SLOT(processUserLeftEvent(const Event_UserLeft &)));
    connect(client, SIGNAL(buddyListReceived(const QList<ServerInfo_User> &)), this, SLOT(buddyListReceived(const QList<ServerInfo_User> &)));
    connect(client, SIGNAL(ignoreListReceived(const QList<ServerInfo_User> &)), this, SLOT(ignoreListReceived(const QList<ServerInfo_User> &)));
    connect(client, SIGNAL(addToListEventReceived(const Event_AddToList &)), this, SLOT(processAddToListEvent(const Event_AddToList &)));
    connect(client, SIGNAL(removeFromListEventReceived(const Event_RemoveFromList &)), this, SLOT(processRemoveFromListEvent(const Event_RemoveFromList &)));
    
    PendingCommand *pend = client->prepareSessionCommand(Command_ListUsers());
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(processListUsersResponse(const Response &)));
    client->sendCommand(pend);
    
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(userInfoBox);
    vbox->addWidget(allUsersList);
    
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(buddyList);
    mainLayout->addWidget(ignoreList);
    mainLayout->addLayout(vbox);
    
    setLayout(mainLayout);
}

void TabUserLists::retranslateUi()
{
    allUsersList->retranslateUi();
    buddyList->retranslateUi();
    ignoreList->retranslateUi();
    userInfoBox->retranslateUi();
}

void TabUserLists::processListUsersResponse(const Response &response)
{
    const Response_ListUsers &resp = response.GetExtension(Response_ListUsers::ext);
    
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

void TabUserLists::processUserJoinedEvent(const Event_UserJoined &event)
{
    const ServerInfo_User &info = event.user_info();
    const QString userName = QString::fromStdString(info.name());
    
    allUsersList->processUserInfo(info, true);
    ignoreList->setUserOnline(userName, true);
    buddyList->setUserOnline(userName, true);
    
    allUsersList->sortItems();
    ignoreList->sortItems();
    buddyList->sortItems();
    
    emit userJoined(info);
}

void TabUserLists::processUserLeftEvent(const Event_UserLeft &event)
{
    QString userName = QString::fromStdString(event.name());
    if (allUsersList->deleteUser(userName)) {
        ignoreList->setUserOnline(userName, false);
        buddyList->setUserOnline(userName, false);
        ignoreList->sortItems();
        buddyList->sortItems();
        
        emit userLeft(userName);
    }
}

void TabUserLists::buddyListReceived(const QList<ServerInfo_User> &_buddyList)
{
    for (int i = 0; i < _buddyList.size(); ++i)
        buddyList->processUserInfo(_buddyList[i], false);
    buddyList->sortItems();
}

void TabUserLists::ignoreListReceived(const QList<ServerInfo_User> &_ignoreList)
{
    for (int i = 0; i < _ignoreList.size(); ++i)
        ignoreList->processUserInfo(_ignoreList[i], false);
    ignoreList->sortItems();
}

void TabUserLists::processAddToListEvent(const Event_AddToList &event)
{
    const ServerInfo_User &info = event.user_info();
    bool online = allUsersList->getUsers().contains(QString::fromStdString(info.name()));
    QString list = QString::fromStdString(event.list_name());
    UserList *userList = 0;
    if (list == "buddy")
        userList = buddyList;
    else if (list == "ignore")
        userList = ignoreList;
    if (!userList)
        return;
    
    userList->processUserInfo(info, online);
    userList->sortItems();
}

void TabUserLists::processRemoveFromListEvent(const Event_RemoveFromList &event)
{
    QString list = QString::fromStdString(event.list_name());
    QString user = QString::fromStdString(event.user_name());
    UserList *userList = 0;
    if (list == "buddy")
        userList = buddyList;
    else if (list == "ignore")
        userList = ignoreList;
    if (!userList)
        return;
    userList->deleteUser(user);
}
