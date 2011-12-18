#include "tab_userlists.h"
#include "userlist.h"
#include "userinfobox.h"
#include "abstractclient.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol_items.h"

#include "pending_command.h"
#include "pb/session_commands.pb.h"

TabUserLists::TabUserLists(TabSupervisor *_tabSupervisor, AbstractClient *_client, ServerInfo_User *userInfo, QWidget *parent)
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
	
	connect(client, SIGNAL(userJoinedEventReceived(Event_UserJoined *)), this, SLOT(processUserJoinedEvent(Event_UserJoined *)));
	connect(client, SIGNAL(userLeftEventReceived(Event_UserLeft *)), this, SLOT(processUserLeftEvent(Event_UserLeft *)));
	connect(client, SIGNAL(buddyListReceived(const QList<ServerInfo_User *> &)), this, SLOT(buddyListReceived(const QList<ServerInfo_User *> &)));
	connect(client, SIGNAL(ignoreListReceived(const QList<ServerInfo_User *> &)), this, SLOT(ignoreListReceived(const QList<ServerInfo_User *> &)));
	connect(client, SIGNAL(addToListEventReceived(Event_AddToList *)), this, SLOT(processAddToListEvent(Event_AddToList *)));
	connect(client, SIGNAL(removeFromListEventReceived(Event_RemoveFromList *)), this, SLOT(processRemoveFromListEvent(Event_RemoveFromList *)));
	
	PendingCommand *pend = client->prepareSessionCommand(Command_ListUsers());
	connect(pend, SIGNAL(finished(ProtocolResponse *)), this, SLOT(processListUsersResponse(ProtocolResponse *)));
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

void TabUserLists::processListUsersResponse(ProtocolResponse *response)
{
	Response_ListUsers *resp = qobject_cast<Response_ListUsers *>(response);
	if (!resp)
		return;
	
	const QList<ServerInfo_User *> &respList = resp->getUserList();
	for (int i = 0; i < respList.size(); ++i) {
		allUsersList->processUserInfo(respList[i], true);
		ignoreList->setUserOnline(respList[i]->getName(), true);
		buddyList->setUserOnline(respList[i]->getName(), true);
	}
	
	allUsersList->sortItems();
	ignoreList->sortItems();
	buddyList->sortItems();
}

void TabUserLists::processUserJoinedEvent(Event_UserJoined *event)
{
	ServerInfo_User *info = event->getUserInfo();
	allUsersList->processUserInfo(info, true);
	ignoreList->setUserOnline(info->getName(), true);
	buddyList->setUserOnline(info->getName(), true);
	
	allUsersList->sortItems();
	ignoreList->sortItems();
	buddyList->sortItems();
	
	emit userJoined(event->getUserInfo()->getName());
}

void TabUserLists::processUserLeftEvent(Event_UserLeft *event)
{
	QString userName = event->getUserName();
	if (allUsersList->deleteUser(userName)) {
		ignoreList->setUserOnline(userName, false);
		buddyList->setUserOnline(userName, false);
		ignoreList->sortItems();
		buddyList->sortItems();
		
		emit userLeft(userName);
	}
}

void TabUserLists::buddyListReceived(const QList<ServerInfo_User *> &_buddyList)
{
	for (int i = 0; i < _buddyList.size(); ++i)
		buddyList->processUserInfo(_buddyList[i], false);
	buddyList->sortItems();
}

void TabUserLists::ignoreListReceived(const QList<ServerInfo_User *> &_ignoreList)
{
	for (int i = 0; i < _ignoreList.size(); ++i)
		ignoreList->processUserInfo(_ignoreList[i], false);
	ignoreList->sortItems();
}

void TabUserLists::processAddToListEvent(Event_AddToList *event)
{
	ServerInfo_User *info = event->getUserInfo();
	bool online = allUsersList->userInList(info->getName());
	QString list = event->getList();
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

void TabUserLists::processRemoveFromListEvent(Event_RemoveFromList *event)
{
	QString list = event->getList();
	QString user = event->getUserName();
	UserList *userList = 0;
	if (list == "buddy")
		userList = buddyList;
	else if (list == "ignore")
		userList = ignoreList;
	if (!userList)
		return;
	userList->deleteUser(user);
}
