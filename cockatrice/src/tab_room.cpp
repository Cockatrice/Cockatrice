#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QLabel>
#include <QSplitter>
#include "tab_supervisor.h"
#include "tab_room.h"
#include "userlist.h"
#include "abstractclient.h"
#include "protocol_items.h"
#include "chatview.h"
#include "gameselector.h"

TabRoom::TabRoom(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &_ownName, ServerInfo_Room *info)
	: Tab(_tabSupervisor), client(_client), roomId(info->getRoomId()), roomName(info->getName()), ownName(_ownName)
{
	const QList<ServerInfo_GameType *> gameTypeList = info->getGameTypeList();
	for (int i = 0; i < gameTypeList.size(); ++i)
		gameTypes.insert(gameTypeList[i]->getGameTypeId(), gameTypeList[i]->getDescription());
	
	QMap<int, GameTypeMap> tempMap;
	tempMap.insert(info->getRoomId(), gameTypes);
	gameSelector = new GameSelector(client, this, QMap<int, QString>(), tempMap);
	userList = new UserList(tabSupervisor, client, UserList::RoomList);
	connect(userList, SIGNAL(openMessageDialog(const QString &, bool)), this, SIGNAL(openMessageDialog(const QString &, bool)));
	
	chatView = new ChatView(ownName, true);
	connect(chatView, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
	connect(chatView, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
	sayLabel = new QLabel;
	sayEdit = new QLineEdit;
	sayLabel->setBuddy(sayEdit);
	connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	
	QHBoxLayout *sayHbox = new QHBoxLayout;
	sayHbox->addWidget(sayLabel);
	sayHbox->addWidget(sayEdit);
	
	QVBoxLayout *chatVbox = new QVBoxLayout;
	chatVbox->addWidget(chatView);
	chatVbox->addLayout(sayHbox);
	
	chatGroupBox = new QGroupBox;
	chatGroupBox->setLayout(chatVbox);
	
	QSplitter *splitter = new QSplitter(Qt::Vertical);
	splitter->addWidget(gameSelector);
	splitter->addWidget(chatGroupBox);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(splitter, 3);
	hbox->addWidget(userList, 1);
	
	aLeaveRoom = new QAction(this);
	connect(aLeaveRoom, SIGNAL(triggered()), this, SLOT(actLeaveRoom()));

	tabMenu = new QMenu(this);
	tabMenu->addAction(aLeaveRoom);

	retranslateUi();
	setLayout(hbox);
	
	const QList<ServerInfo_User *> users = info->getUserList();
	for (int i = 0; i < users.size(); ++i)
		userList->processUserInfo(users[i], true);
	userList->sortItems();
	
	const QList<ServerInfo_Game *> games = info->getGameList();
	for (int i = 0; i < games.size(); ++i)
		gameSelector->processGameInfo(games[i]);
}

TabRoom::~TabRoom()
{
	emit roomClosing(this);
}

void TabRoom::retranslateUi()
{
        gameSelector->retranslateUi();
	sayLabel->setText(tr("&Say:"));
	chatGroupBox->setTitle(tr("Chat"));
	tabMenu->setTitle(tr("&Room"));
	aLeaveRoom->setText(tr("&Leave room"));
}

void TabRoom::closeRequest()
{
	actLeaveRoom();
}

QString TabRoom::sanitizeHtml(QString dirty) const
{
	return dirty
		.replace("&", "&amp;")
		.replace("<", "&lt;")
		.replace(">", "&gt;");
}

void TabRoom::sendMessage()
{
	if (sayEdit->text().isEmpty())
	  	return;
	
	Command_RoomSay *cmd = new Command_RoomSay(roomId, sayEdit->text());
	connect(cmd, SIGNAL(finished(ProtocolResponse *)), this, SLOT(sayFinished(ProtocolResponse *)));
	client->sendCommand(cmd);
	sayEdit->clear();
}

void TabRoom::sayFinished(ProtocolResponse *response)
{
	if (response->getResponseCode() == RespChatFlood)
		chatView->appendMessage(QString(), tr("You are flooding the chat. Please wait a couple of seconds."));
}

void TabRoom::actLeaveRoom()
{
	client->sendCommand(new Command_LeaveRoom(roomId));
	deleteLater();
}

void TabRoom::processRoomEvent(RoomEvent *event)
{
	switch (event->getItemId()) {
		case ItemId_Event_ListGames: processListGamesEvent(qobject_cast<Event_ListGames *>(event)); break;
		case ItemId_Event_JoinRoom: processJoinRoomEvent(qobject_cast<Event_JoinRoom *>(event)); break;
		case ItemId_Event_LeaveRoom: processLeaveRoomEvent(qobject_cast<Event_LeaveRoom *>(event)); break;
		case ItemId_Event_RoomSay: processSayEvent(qobject_cast<Event_RoomSay *>(event)); break;
		default: ;
	}
}

void TabRoom::processListGamesEvent(Event_ListGames *event)
{
	const QList<ServerInfo_Game *> &gameList = event->getGameList();
	for (int i = 0; i < gameList.size(); ++i)
		gameSelector->processGameInfo(gameList[i]);
}

void TabRoom::processJoinRoomEvent(Event_JoinRoom *event)
{
	userList->processUserInfo(event->getUserInfo(), true);
	userList->sortItems();
}

void TabRoom::processLeaveRoomEvent(Event_LeaveRoom *event)
{
	userList->deleteUser(event->getPlayerName());
}

void TabRoom::processSayEvent(Event_RoomSay *event)
{
	chatView->appendMessage(event->getPlayerName(), event->getMessage());
	emit userEvent(false);
}
