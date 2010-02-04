#include <QtGui>
#include "tab_chatchannel.h"
#include "client.h"
#include "protocol_items.h"

TabChatChannel::TabChatChannel(Client *_client, const QString &_channelName)
	: Tab(), client(_client), channelName(_channelName)
{
	playerList = new QListWidget;
	playerList->setFixedWidth(150);
	
	textEdit = new QTextEdit;
	textEdit->setReadOnly(true);
	sayEdit = new QLineEdit;
	connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(textEdit);
	vbox->addWidget(sayEdit);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(vbox);
	hbox->addWidget(playerList);
	
	aLeaveChannel = new QAction(this);
	connect(aLeaveChannel, SIGNAL(triggered()), this, SLOT(actLeaveChannel()));

	tabMenu = new QMenu(this);
	tabMenu->addAction(aLeaveChannel);

	retranslateUi();
	setLayout(hbox);
}

TabChatChannel::~TabChatChannel()
{
	emit channelClosing(this);
}

void TabChatChannel::retranslateUi()
{
	tabMenu->setTitle(tr("C&hat channel"));
	aLeaveChannel->setText(tr("&Leave channel"));
}

void TabChatChannel::sendMessage()
{
	if (sayEdit->text().isEmpty())
	  	return;
	
	client->sendCommand(new Command_ChatSay(channelName, sayEdit->text()));
	sayEdit->clear();
}

void TabChatChannel::actLeaveChannel()
{
	client->sendCommand(new Command_ChatLeaveChannel(channelName));
	deleteLater();
}

void TabChatChannel::processChatEvent(ChatEvent *event)
{
	switch (event->getItemId()) {
		case ItemId_Event_ChatListPlayers: processListPlayersEvent(qobject_cast<Event_ChatListPlayers *>(event)); break;
		case ItemId_Event_ChatJoinChannel: processJoinChannelEvent(qobject_cast<Event_ChatJoinChannel *>(event)); break;
		case ItemId_Event_ChatLeaveChannel: processLeaveChannelEvent(qobject_cast<Event_ChatLeaveChannel *>(event)); break;
		case ItemId_Event_ChatSay: processSayEvent(qobject_cast<Event_ChatSay *>(event)); break;
		default: ;
	}
}

void TabChatChannel::processListPlayersEvent(Event_ChatListPlayers *event)
{
	const QList<ServerInfo_ChatUser *> &players = event->getPlayerList();
	for (int i = 0; i < players.size(); ++i)
		playerList->addItem(players[i]->getName());
}

void TabChatChannel::processJoinChannelEvent(Event_ChatJoinChannel *event)
{
	textEdit->append(tr("%1 has joined the channel.").arg(event->getPlayerName()));
	playerList->addItem(event->getPlayerName());
	emit userEvent();
}

void TabChatChannel::processLeaveChannelEvent(Event_ChatLeaveChannel *event)
{
	textEdit->append(tr("%1 has left the channel.").arg(event->getPlayerName()));
	for (int i = 0; i < playerList->count(); ++i)
		if (playerList->item(i)->text() == event->getPlayerName()) {
			delete playerList->takeItem(i);
			break;
		}
	emit userEvent();
}

void TabChatChannel::processSayEvent(Event_ChatSay *event)
{
	if (event->getPlayerName().isEmpty())
		textEdit->append(QString("<font color=\"blue\">%1</font").arg(event->getMessage()));
	else
		textEdit->append(QString("<font color=\"red\">%1:</font> %2").arg(event->getPlayerName()).arg(event->getMessage()));
	emit userEvent();
}
