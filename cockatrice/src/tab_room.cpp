#include <QTextEdit>
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
#include <QScrollBar>
#include "dlg_creategame.h"
#include "tab_room.h"
#include "userlist.h"
#include "abstractclient.h"
#include "protocol_items.h"
#include "gamesmodel.h"

#include <QTextTable>

GameSelector::GameSelector(AbstractClient *_client, int _roomId, QWidget *parent)
	: QGroupBox(parent), client(_client), roomId(_roomId)
{
	gameListView = new QTreeView;
	gameListModel = new GamesModel(this);
	gameListProxyModel = new GamesProxyModel(this);
	gameListProxyModel->setSourceModel(gameListModel);
	gameListView->setModel(gameListProxyModel);
	gameListView->header()->setResizeMode(0, QHeaderView::ResizeToContents);

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

	setMinimumWidth(gameListView->columnWidth(0) * gameListModel->columnCount());
	setMinimumHeight(400);

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
	DlgCreateGame dlg(client, roomId, this);
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

	Command_JoinGame *commandJoinGame = new Command_JoinGame(roomId, game->getGameId(), password, spectator);
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

ChatView::ChatView(const QString &_ownName, QWidget *parent)
	: QTextEdit(parent), ownName(_ownName)
{
	setTextInteractionFlags(Qt::TextSelectableByMouse);
	
	QTextTableFormat format;
	format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
	table = textCursor().insertTable(1, 3, format);
}

void ChatView::appendMessage(const QString &sender, const QString &message)
{
	QTextCursor cellCursor = table->cellAt(table->rows() - 1, 0).lastCursorPosition();
	cellCursor.insertText(QDateTime::currentDateTime().toString("[hh:mm]"));
	QTextTableCell senderCell = table->cellAt(table->rows() - 1, 1);
	QTextCharFormat senderFormat;
	if (sender == ownName) {
		senderFormat.setFontWeight(QFont::Bold);
		senderFormat.setForeground(Qt::red);
	} else
		senderFormat.setForeground(Qt::blue);
	senderCell.setFormat(senderFormat);
	cellCursor = senderCell.lastCursorPosition();
	cellCursor.insertText(sender);
	QTextTableCell messageCell = table->cellAt(table->rows() - 1, 2);
	QTextCharFormat messageFormat;
	if (sender.isEmpty())
		messageFormat.setForeground(Qt::darkGreen);
	messageCell.setFormat(messageFormat);
	cellCursor = messageCell.lastCursorPosition();
	cellCursor.insertText(message);
	
	table->appendRows(1);
	
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

TabRoom::TabRoom(AbstractClient *_client, const QString &_ownName, ServerInfo_Room *info)
	: Tab(), client(_client), roomId(info->getRoomId()), roomName(info->getName()), ownName(_ownName)
{
	gameSelector = new GameSelector(client, roomId);
	userList = new UserList(client, false);
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
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(gameSelector);
	vbox->addWidget(chatGroupBox);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(vbox, 3);
	hbox->addWidget(userList, 1);
	
	aLeaveRoom = new QAction(this);
	connect(aLeaveRoom, SIGNAL(triggered()), this, SLOT(actLeaveRoom()));

	tabMenu = new QMenu(this);
	tabMenu->addAction(aLeaveRoom);

	retranslateUi();
	setLayout(hbox);
	
	const QList<ServerInfo_User *> users = info->getUserList();
	for (int i = 0; i < users.size(); ++i)
		userList->processUserInfo(users[i]);
	
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
	
	client->sendCommand(new Command_RoomSay(roomId, sayEdit->text()));
	sayEdit->clear();
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
	chatView->appendMessage(QString(), tr("%1 has joined the room.").arg(event->getUserInfo()->getName()));
	userList->processUserInfo(event->getUserInfo());
}

void TabRoom::processLeaveRoomEvent(Event_LeaveRoom *event)
{
	chatView->appendMessage(QString(), tr("%1 has left the room.").arg(event->getPlayerName()));
	userList->deleteUser(event->getPlayerName());
}

void TabRoom::processSayEvent(Event_RoomSay *event)
{
	chatView->appendMessage(event->getPlayerName(), event->getMessage());
	emit userEvent();
}
