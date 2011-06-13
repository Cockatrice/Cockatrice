#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QCheckBox>
#include <QInputDialog>
#include <QLabel>
#include <QSplitter>
#include "dlg_creategame.h"
#include "tab_supervisor.h"
#include "tab_room.h"
#include "userlist.h"
#include "abstractclient.h"
#include "protocol_items.h"
#include "gamesmodel.h"
#include "chatview.h"

GameSelector::GameSelector(AbstractClient *_client, TabRoom *_room, QWidget *parent)
	: QGroupBox(parent), client(_client), room(_room)
{
	gameListView = new QTreeView;
	gameListModel = new GamesModel(room->getGameTypes(), this);
	gameListProxyModel = new GamesProxyModel(this);
	gameListProxyModel->setSourceModel(gameListModel);
	gameListProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	gameListView->setModel(gameListProxyModel);
	gameListView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	gameListView->setSortingEnabled(true);

	showFullGamesCheckBox = new QCheckBox;
	createButton = new QPushButton;
	joinButton = new QPushButton;
	spectateButton = new QPushButton;
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(showFullGamesCheckBox);
	buttonLayout->addStretch();
	buttonLayout->addWidget(createButton);
	buttonLayout->addWidget(joinButton);
	buttonLayout->addWidget(spectateButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(gameListView);
	mainLayout->addLayout(buttonLayout);

	retranslateUi();
	setLayout(mainLayout);

	setMinimumWidth((qreal) (gameListView->columnWidth(0) * gameListModel->columnCount()) / 1.5);
	setMinimumHeight(200);

	connect(showFullGamesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showFullGamesChanged(int)));
	connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
	connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));
	connect(spectateButton, SIGNAL(clicked()), this, SLOT(actJoin()));
}

void GameSelector::showFullGamesChanged(int state)
{
	gameListProxyModel->setFullGamesVisible(state);
}

void GameSelector::actCreate()
{
	DlgCreateGame dlg(client, room->getRoomId(), room->getGameTypes(), this);
	dlg.exec();
}

void GameSelector::checkResponse(ResponseCode response)
{
	createButton->setEnabled(true);
	joinButton->setEnabled(true);
	spectateButton->setEnabled(true);

	switch (response) {
		case RespWrongPassword: QMessageBox::critical(this, tr("Error"), tr("Wrong password.")); break;
		case RespSpectatorsNotAllowed: QMessageBox::critical(this, tr("Error"), tr("Spectators are not allowed in this game.")); break;
		case RespGameFull: QMessageBox::critical(this, tr("Error"), tr("The game is already full.")); break;
		case RespNameNotFound: QMessageBox::critical(this, tr("Error"), tr("The game does not exist any more.")); break;
		case RespUserLevelTooLow: QMessageBox::critical(this, tr("Error"), tr("This game is only open to registered users.")); break;
		case RespOnlyBuddies: QMessageBox::critical(this, tr("Error"), tr("This game is only open to its creator's buddies.")); break;
		case RespInIgnoreList: QMessageBox::critical(this, tr("Error"), tr("You are being ignored by the creator of this game.")); break;
		default: ;
	}
}

void GameSelector::actJoin()
{
	bool spectator = sender() == spectateButton;
	
	QModelIndex ind = gameListView->currentIndex();
	if (!ind.isValid())
		return;
	ServerInfo_Game *game = gameListModel->getGame(ind.data(Qt::UserRole).toInt());
	QString password;
	if (game->getHasPassword() && !(spectator && !game->getSpectatorsNeedPassword())) {
		bool ok;
		password = QInputDialog::getText(this, tr("Join game"), tr("Password:"), QLineEdit::Password, QString(), &ok);
		if (!ok)
			return;
	}

	Command_JoinGame *commandJoinGame = new Command_JoinGame(room->getRoomId(), game->getGameId(), password, spectator);
	connect(commandJoinGame, SIGNAL(finished(ResponseCode)), this, SLOT(checkResponse(ResponseCode)));
	client->sendCommand(commandJoinGame);

	createButton->setEnabled(false);
	joinButton->setEnabled(false);
	spectateButton->setEnabled(false);
}

void GameSelector::retranslateUi()
{
	setTitle(tr("Games"));
	showFullGamesCheckBox->setText(tr("Show &full games"));
	createButton->setText(tr("C&reate"));
	joinButton->setText(tr("&Join"));
	spectateButton->setText(tr("J&oin as spectator"));
}

void GameSelector::processGameInfo(ServerInfo_Game *info)
{
	gameListModel->updateGameList(info);
}

TabRoom::TabRoom(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &_ownName, ServerInfo_Room *info)
	: Tab(_tabSupervisor), client(_client), roomId(info->getRoomId()), roomName(info->getName()), ownName(_ownName)
{
	const QList<ServerInfo_GameType *> gameTypeList = info->getGameTypeList();
	for (int i = 0; i < gameTypeList.size(); ++i)
		gameTypes.insert(gameTypeList[i]->getGameTypeId(), gameTypeList[i]->getDescription());
	
	gameSelector = new GameSelector(client, this);
	userList = new UserList(tabSupervisor, client, UserList::RoomList);
	connect(userList, SIGNAL(openMessageDialog(const QString &, bool)), this, SIGNAL(openMessageDialog(const QString &, bool)));
	
	chatView = new ChatView(ownName);
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
	emit userEvent();
}
