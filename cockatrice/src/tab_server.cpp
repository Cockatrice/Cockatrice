#include <QtGui>
#include "tab_server.h"
#include "gamesmodel.h"
#include "dlg_creategame.h"
#include "client.h"
#include "protocol.h"
#include "protocol_items.h"

GameSelector::GameSelector(Client *_client, QWidget *parent)
	: QGroupBox(parent), client(_client)
{
	gameListView = new QTreeView;
	gameListModel = new GamesModel(this);
	gameListProxyModel = new GamesProxyModel(this);
	gameListProxyModel->setSourceModel(gameListModel);
	gameListView->setModel(gameListProxyModel);

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

	connect(client, SIGNAL(listGamesEventReceived(Event_ListGames *)), this, SLOT(processListGamesEvent(Event_ListGames *)));
	client->sendCommand(new Command_ListGames);
}

void GameSelector::showFullGamesChanged(int state)
{
	gameListProxyModel->setFullGamesVisible(state);
}

void GameSelector::actCreate()
{
	DlgCreateGame dlg(client, this);
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
	if (game->getHasPassword()) {
		bool ok;
		password = QInputDialog::getText(this, tr("Join game"), tr("Password:"), QLineEdit::Password, QString(), &ok);
		if (!ok)
			return;
	}

	Command_JoinGame *commandJoinGame = new Command_JoinGame(game->getGameId(), password, spectator);
	connect(commandJoinGame, SIGNAL(finished(ResponseCode)), this, SLOT(checkResponse(ResponseCode)));
	client->sendCommand(commandJoinGame);

	createButton->setEnabled(false);
	joinButton->setEnabled(false);
	spectateButton->setEnabled(false);
}

void GameSelector::retranslateUi()
{
	setTitle(tr("Games"));
	showFullGamesCheckBox->setText(tr("&Show full games"));
	createButton->setText(tr("C&reate"));
	joinButton->setText(tr("&Join"));
	spectateButton->setText(tr("J&oin as spectator"));
}

void GameSelector::processListGamesEvent(Event_ListGames *event)
{
	const QList<ServerInfo_Game *> &gamesToUpdate = event->getGameList();
	for (int i = 0; i < gamesToUpdate.size(); ++i)
		gameListModel->updateGameList(gamesToUpdate[i]);
}

ChatChannelSelector::ChatChannelSelector(Client *_client, QWidget *parent)
	: QGroupBox(parent), client(_client)
{
	channelList = new QTreeWidget;
	channelList->setRootIsDecorated(false);
	
	joinButton = new QPushButton;
	connect(joinButton, SIGNAL(clicked()), this, SLOT(joinClicked()));
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(joinButton);
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(channelList);
	vbox->addLayout(buttonLayout);
	
	retranslateUi();
	setLayout(vbox);
	
	connect(client, SIGNAL(listChatChannelsEventReceived(Event_ListChatChannels *)), this, SLOT(processListChatChannelsEvent(Event_ListChatChannels *)));
	client->sendCommand(new Command_ListChatChannels);
}

void ChatChannelSelector::retranslateUi()
{
	setTitle(tr("Chat channels"));
	joinButton->setText(tr("Joi&n"));

	QTreeWidgetItem *header = channelList->headerItem();
	header->setText(0, tr("Channel"));
	header->setText(1, tr("Description"));
	header->setText(2, tr("Players"));
	header->setTextAlignment(2, Qt::AlignRight);
}

void ChatChannelSelector::processListChatChannelsEvent(Event_ListChatChannels *event)
{
	const QList<ServerInfo_ChatChannel *> &channelsToUpdate = event->getChannelList();
	for (int i = 0; i < channelsToUpdate.size(); ++i) {
		ServerInfo_ChatChannel *channel = channelsToUpdate[i];
		
		for (int j = 0; j < channelList->topLevelItemCount(); ++j) {
		  	QTreeWidgetItem *twi = channelList->topLevelItem(j);
			if (twi->text(0) == channel->getName()) {
				twi->setText(1, channel->getDescription());
				twi->setText(2, QString::number(channel->getPlayerCount()));
				return;
			}
		}
		QTreeWidgetItem *twi = new QTreeWidgetItem(QStringList() << channel->getName() << channel->getDescription() << QString::number(channel->getPlayerCount()));
		twi->setTextAlignment(2, Qt::AlignRight);
		channelList->addTopLevelItem(twi);
		channelList->resizeColumnToContents(0);
		channelList->resizeColumnToContents(1);
		channelList->resizeColumnToContents(2);
		if (channel->getAutoJoin())
			joinChannel(channel->getName());
	}
}

void ChatChannelSelector::joinChannel(const QString &channelName)
{
	Command_ChatJoinChannel *command = new Command_ChatJoinChannel(channelName);
	connect(command, SIGNAL(finished(ResponseCode)), this, SLOT(joinFinished(ResponseCode)));
	client->sendCommand(command);
}

void ChatChannelSelector::joinClicked()
{
	QTreeWidgetItem *twi = channelList->currentItem();
	if (!twi)
		return;
	QString channelName = twi->text(0);
	
	joinChannel(channelName);
}

void ChatChannelSelector::joinFinished(ResponseCode resp)
{
	if (resp != RespOk)
		return;
	
	Command_ChatJoinChannel *command = qobject_cast<Command_ChatJoinChannel *>(sender());
	QString channelName = command->getChannel();
	
	emit channelJoined(channelName);
}

ServerMessageLog::ServerMessageLog(Client *_client, QWidget *parent)
	: QGroupBox(parent)
{
	textEdit = new QTextEdit;
	textEdit->setReadOnly(true);
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(textEdit);
	
	setLayout(vbox);
	retranslateUi();

	connect(_client, SIGNAL(serverMessageEventReceived(Event_ServerMessage *)), this, SLOT(processServerMessageEvent(Event_ServerMessage *)));
}

void ServerMessageLog::retranslateUi()
{
	setTitle(tr("Server messages"));
}

void ServerMessageLog::processServerMessageEvent(Event_ServerMessage *event)
{
	textEdit->append(event->getMessage());
}

TabServer::TabServer(Client *_client, QWidget *parent)
	: Tab(parent), client(_client)
{
	gameSelector = new GameSelector(client);
	chatChannelSelector = new ChatChannelSelector(client);
	serverMessageLog = new ServerMessageLog(client);
	
	connect(gameSelector, SIGNAL(gameJoined(int)), this, SIGNAL(gameJoined(int)));
	connect(chatChannelSelector, SIGNAL(channelJoined(const QString &)), this, SIGNAL(chatChannelJoined(const QString &)));
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(chatChannelSelector);
	hbox->addWidget(serverMessageLog);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(gameSelector);
	mainLayout->addLayout(hbox);
	
	setLayout(mainLayout);
}

void TabServer::retranslateUi()
{
	gameSelector->retranslateUi();
	chatChannelSelector->retranslateUi();
	serverMessageLog->retranslateUi();
}
