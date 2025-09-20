#include "tab_account.h"

#include "../../deck/custom_line_edit.h"
#include "../../server/abstract_client.h"
#include "../../server/pending_command.h"
#include "../../server/user/user_info_box.h"
#include "../../server/user/user_list_manager.h"
#include "../../server/user/user_list_widget.h"
#include "../sound_engine.h"
#include "pb/event_add_to_list.pb.h"
#include "pb/event_remove_from_list.pb.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include "pb/response_list_users.pb.h"
#include "pb/session_commands.pb.h"
#include "tab_supervisor.h"
#include "trice_limits.h"

#include <QPushButton>
#include <QVBoxLayout>

TabAccount::TabAccount(TabSupervisor *_tabSupervisor, AbstractClient *_client, const ServerInfo_User &userInfo)
    : Tab(_tabSupervisor), client(_client)
{
    allUsersList = new UserListWidget(_tabSupervisor, client, UserListWidget::AllUsersList);
    buddyList = new UserListWidget(_tabSupervisor, client, UserListWidget::BuddyList);
    ignoreList = new UserListWidget(_tabSupervisor, client, UserListWidget::IgnoreList);
    userInfoBox = new UserInfoBox(client, true);
    userInfoBox->updateInfo(userInfo);

    connect(allUsersList, &UserListWidget::openMessageDialog, this, &TabAccount::openMessageDialog);
    connect(buddyList, &UserListWidget::openMessageDialog, this, &TabAccount::openMessageDialog);
    connect(ignoreList, &UserListWidget::openMessageDialog, this, &TabAccount::openMessageDialog);

    connect(client, &AbstractClient::userJoinedEventReceived, this, &TabAccount::processUserJoinedEvent);
    connect(client, &AbstractClient::userLeftEventReceived, this, &TabAccount::processUserLeftEvent);
    connect(client, &AbstractClient::buddyListReceived, this, &TabAccount::buddyListReceived);
    connect(client, &AbstractClient::ignoreListReceived, this, &TabAccount::ignoreListReceived);
    connect(client, &AbstractClient::addToListEventReceived, this, &TabAccount::processAddToListEvent);
    connect(client, &AbstractClient::removeFromListEventReceived, this, &TabAccount::processRemoveFromListEvent);

    // Attempt to populate the tab with the cache
    buddyListReceived(tabSupervisor->getUserListManager()->getBuddyList().values());
    ignoreListReceived(tabSupervisor->getUserListManager()->getIgnoreList().values());

    PendingCommand *pend = AbstractClient::prepareSessionCommand(Command_ListUsers());
    connect(pend, &PendingCommand::finished, this, &TabAccount::processListUsersResponse);
    client->sendCommand(pend);

    auto *vbox = new QVBoxLayout;
    vbox->addWidget(userInfoBox);
    vbox->addWidget(allUsersList);

    auto *addToBuddyList = new QHBoxLayout;
    addBuddyEdit = new LineEditUnfocusable;
    addBuddyEdit->setMaxLength(MAX_NAME_LENGTH);
    addBuddyEdit->setPlaceholderText(tr("Add to Buddy List"));
    connect(addBuddyEdit, &LineEditUnfocusable::returnPressed, this, &TabAccount::addToBuddyList);
    auto *addBuddyButton = new QPushButton("Add");
    connect(addBuddyButton, &QPushButton::clicked, this, &TabAccount::addToBuddyList);
    addToBuddyList->addWidget(addBuddyEdit);
    addToBuddyList->addWidget(addBuddyButton);

    auto *addToIgnoreList = new QHBoxLayout;
    addIgnoreEdit = new LineEditUnfocusable;
    addIgnoreEdit->setMaxLength(MAX_NAME_LENGTH);
    addIgnoreEdit->setPlaceholderText(tr("Add to Ignore List"));
    connect(addIgnoreEdit, &LineEditUnfocusable::returnPressed, this, &TabAccount::addToIgnoreList);
    auto *addIgnoreButton = new QPushButton("Add");
    connect(addIgnoreButton, &QPushButton::clicked, this, &TabAccount::addToIgnoreList);
    addToIgnoreList->addWidget(addIgnoreEdit);
    addToIgnoreList->addWidget(addIgnoreButton);

    auto *buddyPanel = new QVBoxLayout;
    buddyPanel->addWidget(buddyList);
    buddyPanel->addLayout(addToBuddyList);

    auto *ignorePanel = new QVBoxLayout;
    ignorePanel->addWidget(ignoreList);
    ignorePanel->addLayout(addToIgnoreList);

    auto *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(buddyPanel);
    mainLayout->addLayout(ignorePanel);
    mainLayout->addLayout(vbox);

    retranslateUi();

    auto *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
}

void TabAccount::addToBuddyList()
{
    const QString &userName = addBuddyEdit->text();
    if (userName.isEmpty()) {
        return;
    }

    const std::string listName = "buddy";
    addToList(listName, userName);
    addBuddyEdit->clear();
}

void TabAccount::addToIgnoreList()
{
    const QString &userName = addIgnoreEdit->text();
    if (userName.isEmpty()) {
        return;
    }

    const std::string listName = "ignore";
    addToList(listName, userName);
    addIgnoreEdit->clear();
}

void TabAccount::addToList(const std::string &listName, const QString &userName)
{
    Command_AddToList cmd;
    cmd.set_list(listName);
    cmd.set_user_name(userName.toStdString());

    client->sendCommand(AbstractClient::prepareSessionCommand(cmd));
}

void TabAccount::retranslateUi()
{
    allUsersList->retranslateUi();
    buddyList->retranslateUi();
    ignoreList->retranslateUi();
    userInfoBox->retranslateUi();
}

void TabAccount::processListUsersResponse(const Response &response)
{
    const Response_ListUsers &resp = response.GetExtension(Response_ListUsers::ext);
    for (int i = 0; i < resp.user_list_size(); ++i) {
        const ServerInfo_User &info = resp.user_list(i);
        const QString &userName = QString::fromStdString(info.name());
        allUsersList->processUserInfo(info, true);
        ignoreList->setUserOnline(userName, true);
        buddyList->setUserOnline(userName, true);
    }

    allUsersList->sortItems();
    ignoreList->sortItems();
    buddyList->sortItems();
}

void TabAccount::processUserJoinedEvent(const Event_UserJoined &event)
{
    const ServerInfo_User &info = event.user_info();
    const QString &userName = QString::fromStdString(info.name());

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

void TabAccount::processUserLeftEvent(const Event_UserLeft &event)
{
    const QString &userName = QString::fromStdString(event.name());

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

void TabAccount::buddyListReceived(const QList<ServerInfo_User> &_buddyList)
{
    for (const auto &user : _buddyList) {
        buddyList->processUserInfo(user, false);
    }
    buddyList->sortItems();
}

void TabAccount::ignoreListReceived(const QList<ServerInfo_User> &_ignoreList)
{
    for (const auto &user : _ignoreList) {
        ignoreList->processUserInfo(user, false);
    }
    ignoreList->sortItems();
}

void TabAccount::processAddToListEvent(const Event_AddToList &event)
{
    const ServerInfo_User &info = event.user_info();
    const bool online = allUsersList->getUsers().contains(QString::fromStdString(info.name()));
    const QString &list = QString::fromStdString(event.list_name());

    UserListWidget *userList;
    if (list == "buddy") {
        userList = buddyList;
    } else if (list == "ignore") {
        userList = ignoreList;
    } else {
        return;
    }

    userList->processUserInfo(info, online);
    userList->sortItems();
}

void TabAccount::processRemoveFromListEvent(const Event_RemoveFromList &event)
{
    const auto &list = QString::fromStdString(event.list_name());
    const auto &user = QString::fromStdString(event.user_name());

    UserListWidget *userList;
    if (list == "buddy") {
        userList = buddyList;
    } else if (list == "ignore") {
        userList = ignoreList;
    } else {
        return;
    }

    userList->deleteUser(user);
}
