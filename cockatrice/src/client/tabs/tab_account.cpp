#include "tab_account.h"

#include "../../deck/custom_line_edit.h"
#include "../../server/pending_command.h"
#include "../../server/user/user_info_box.h"
#include "../../server/user/user_list.h"
#include "../game_logic/abstract_client.h"
#include "../sound_engine.h"
#include "../user_list_manager.h"
#include "pb/event_add_to_list.pb.h"
#include "pb/event_remove_from_list.pb.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include "pb/response_list_users.pb.h"
#include "pb/session_commands.pb.h"
#include "trice_limits.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

TabUserLists::TabUserLists(TabSupervisor *_tabSupervisor,
                           AbstractClient *_client,
                           const ServerInfo_User &userInfo,
                           UserListManager *_userListManager)
    : Tab(_tabSupervisor), client(_client), userListManager(_userListManager)
{
    userInfoBox = new UserInfoBox(client, true);
    userInfoBox->updateInfo(userInfo);

    connect(userListManager->getAllUsersList(), &UserList::openMessageDialog, this, &TabUserLists::openMessageDialog);
    connect(userListManager->getBuddyList(), &UserList::openMessageDialog, this, &TabUserLists::openMessageDialog);
    connect(userListManager->getIgnoreList(), &UserList::openMessageDialog, this, &TabUserLists::openMessageDialog);

    PendingCommand *pend = client->prepareSessionCommand(Command_ListUsers());
    connect(pend,
            static_cast<void (PendingCommand::*)(const Response &, const CommandContainer &, const QVariant &)>(
                &PendingCommand::finished),
            userListManager, &UserListManager::processListUsersResponse);
    client->sendCommand(pend);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(userInfoBox);
    vbox->addWidget(userListManager->getAllUsersList());

    QHBoxLayout *addToBuddyList = new QHBoxLayout;
    addBuddyEdit = new LineEditUnfocusable;
    addBuddyEdit->setMaxLength(MAX_NAME_LENGTH);
    addBuddyEdit->setPlaceholderText(tr("Add to Buddy List"));
    connect(addBuddyEdit, &LineEditUnfocusable::returnPressed, this, &TabUserLists::addToBuddyList);
    QPushButton *addBuddyButton = new QPushButton("Add");
    connect(addBuddyButton, &QPushButton::clicked, this, &TabUserLists::addToBuddyList);
    addToBuddyList->addWidget(addBuddyEdit);
    addToBuddyList->addWidget(addBuddyButton);

    QHBoxLayout *addToIgnoreList = new QHBoxLayout;
    addIgnoreEdit = new LineEditUnfocusable;
    addIgnoreEdit->setMaxLength(MAX_NAME_LENGTH);
    addIgnoreEdit->setPlaceholderText(tr("Add to Ignore List"));
    connect(addIgnoreEdit, &LineEditUnfocusable::returnPressed, this, &TabUserLists::addToIgnoreList);
    QPushButton *addIgnoreButton = new QPushButton("Add");
    connect(addIgnoreButton, &QPushButton::clicked, this, &TabUserLists::addToIgnoreList);
    addToIgnoreList->addWidget(addIgnoreEdit);
    addToIgnoreList->addWidget(addIgnoreButton);

    QVBoxLayout *buddyPanel = new QVBoxLayout;
    buddyPanel->addWidget(userListManager->getBuddyList());
    buddyPanel->addLayout(addToBuddyList);

    QVBoxLayout *ignorePanel = new QVBoxLayout;
    ignorePanel->addWidget(userListManager->getIgnoreList());
    ignorePanel->addLayout(addToIgnoreList);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(buddyPanel);
    mainLayout->addLayout(ignorePanel);
    mainLayout->addLayout(vbox);

    retranslateUi();

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
}

void TabUserLists::addToBuddyList()
{
    QString userName = addBuddyEdit->text();
    if (userName.length() < 1)
        return;

    std::string listName = "buddy";
    userListManager->addToList(listName, userName);
    addBuddyEdit->clear();
}

void TabUserLists::addToIgnoreList()
{
    QString userName = addIgnoreEdit->text();
    if (userName.length() < 1)
        return;

    std::string listName = "ignore";
    userListManager->addToList(listName, userName);
    addIgnoreEdit->clear();
}

void TabUserLists::retranslateUi()
{
    userInfoBox->retranslateUi();
    userListManager->retranslateUi();
}
