#include "tab_userlists.h"
#include "userlist.h"
#include "userinfobox.h"
#include "abstractclient.h"
#include "soundengine.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>

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
    userInfoBox = new UserInfoBox(client, true);
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

    QHBoxLayout *addToBuddyList = new QHBoxLayout;
    addBuddyEdit = new QLineEdit;
    addBuddyEdit->setPlaceholderText(tr("Add to Buddy List"));
    connect(addBuddyEdit, SIGNAL(returnPressed()), this, SLOT(addToBuddyList()));
    QPushButton *addBuddyButton = new QPushButton("Add");
    connect(addBuddyButton, SIGNAL(clicked()), this, SLOT(addToBuddyList()));
    addToBuddyList->addWidget(addBuddyEdit);
    addToBuddyList->addWidget(addBuddyButton);

    QHBoxLayout *addToIgnoreList = new QHBoxLayout;
    addIgnoreEdit = new QLineEdit;
    addIgnoreEdit->setPlaceholderText(tr("Add to Ignore List"));
    connect(addIgnoreEdit, SIGNAL(returnPressed()), this, SLOT(addToIgnoreList()));
    QPushButton *addIgnoreButton = new QPushButton("Add");
    connect(addIgnoreButton, SIGNAL(clicked()), this, SLOT(addToIgnoreList()));
    addToIgnoreList->addWidget(addIgnoreEdit);
    addToIgnoreList->addWidget(addIgnoreButton);

    QVBoxLayout *buddyPanel = new QVBoxLayout;
    buddyPanel->addWidget(buddyList);
    buddyPanel->addLayout(addToBuddyList);

    QVBoxLayout *ignorePanel = new QVBoxLayout;
    ignorePanel->addWidget(ignoreList);
    ignorePanel->addLayout(addToIgnoreList);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(buddyPanel);
    mainLayout->addLayout(ignorePanel);
    mainLayout->addLayout(vbox);

    retranslateUi();

    QWidget * mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
}

void TabUserLists::addToBuddyList()
{
    QString userName = addBuddyEdit->text();
    if (userName.length() < 1) return;

    std::string listName = "buddy";
    addToList(listName, userName);
    addBuddyEdit->clear();
}

void TabUserLists::addToIgnoreList()
{
    QString userName = addIgnoreEdit->text();
    if (userName.length() < 1) return;

    std::string listName = "ignore";
    addToList(listName, userName);
    addIgnoreEdit->clear();
}

void TabUserLists::addToList(const std::string &listName, const QString &userName)
{
    Command_AddToList cmd;
    cmd.set_list(listName);
    cmd.set_user_name(userName.toStdString());

    client->sendCommand(client->prepareSessionCommand(cmd));
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
    
    if (buddyList->getUsers().keys().contains(userName))
        soundEngine->playSound("buddy_join");

    emit userJoined(info);
}

void TabUserLists::processUserLeftEvent(const Event_UserLeft &event)
{
    QString userName = QString::fromStdString(event.name());

    if (buddyList->getUsers().keys().contains(userName))
        soundEngine->playSound("buddy_leave");

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
