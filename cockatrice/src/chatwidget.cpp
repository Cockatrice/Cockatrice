#include <QtGui>
#include <QApplication>
#include "chatwidget.h"
#include "client.h"

ChannelWidget::ChannelWidget(Client *_client, const QString &_name, bool readOnly, bool _virtualChannel, QWidget *parent)
	: QWidget(parent), client(_client), name(_name), virtualChannel(_virtualChannel)
{
	playerList = new QListWidget;
	playerList->setFixedWidth(100);
	
	textEdit = new QTextEdit;
	textEdit->setReadOnly(true);
	if (!readOnly) {
		sayEdit = new QLineEdit;
		connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	}
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(textEdit);
	if (!readOnly)
		vbox->addWidget(sayEdit);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(vbox);
	hbox->addWidget(playerList);
	
	setLayout(hbox);
}

ChannelWidget::~ChannelWidget()
{
  	if (!virtualChannel)
		client->chatLeaveChannel(name);
}

void ChannelWidget::sendMessage()
{
	if (sayEdit->text().isEmpty())
	  	return;
	client->chatSay(name, sayEdit->text());
	sayEdit->clear();
}

void ChannelWidget::joinEvent(const QString &playerName)
{
	textEdit->append(tr("%1 has joined the channel.").arg(playerName));
	playerList->addItem(playerName);
}

void ChannelWidget::listPlayersEvent(const QString &playerName)
{
	playerList->addItem(playerName);
}

void ChannelWidget::leaveEvent(const QString &playerName)
{
	textEdit->append(tr("%1 has left the channel.").arg(playerName));
	for (int i = 0; i < playerList->count(); ++i)
		if (playerList->item(i)->text() == playerName) {
			delete playerList->takeItem(i);
			break;
		}
}

void ChannelWidget::sayEvent(const QString &playerName, const QString &s)
{
	textEdit->append(QString("<font color=\"red\">%1:</font> %2").arg(playerName).arg(s));
	QApplication::alert(this);
}

void ChannelWidget::serverMessageEvent(const QString &s)
{
	textEdit->append(QString("<font color=\"blue\">%1</font>").arg(s));
}

ChatWidget::ChatWidget(Client *_client, QWidget *parent)
	: QWidget(parent), client(_client)
{
	channelList = new QTreeWidget;
	channelList->setRootIsDecorated(false);
	channelList->setFixedWidth(200);
	
	joinButton = new QPushButton;
	connect(joinButton, SIGNAL(clicked()), this, SLOT(joinClicked()));
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(joinButton);
	QVBoxLayout *leftLayout = new QVBoxLayout;
	leftLayout->addWidget(channelList);
	leftLayout->addLayout(buttonLayout);
	
	tab = new QTabWidget;
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(leftLayout);
	hbox->addWidget(tab, 1);
	
	retranslateUi();
	setLayout(hbox);
}

void ChatWidget::retranslateUi()
{
	joinButton->setText(tr("Joi&n"));

	QTreeWidgetItem *header = channelList->headerItem();
	header->setText(0, tr("Channel"));
	header->setText(1, tr("Players"));
	header->setTextAlignment(1, Qt::AlignRight);
}

void ChatWidget::enableChat()
{
	connect(client, SIGNAL(chatEvent(const ChatEventData &)), this, SLOT(chatEvent(const ChatEventData &)));
	client->chatListChannels();
	show();
}

void ChatWidget::disableChat()
{
	disconnect(client, 0, this, 0);
	while (tab->count()) {
		ChannelWidget *cw = qobject_cast<ChannelWidget *>(tab->widget(0));
		tab->removeTab(0);
		delete cw;
	}
	channelList->clear();
	hide();
}

void ChatWidget::chatEvent(const ChatEventData &data)
{
	const QStringList &msg = data.getEventData();
	switch (data.getEventType()) {
		case eventChatListChannels: {
			if (msg.size() != 4)
				break;
			for (int i = 0; i < channelList->topLevelItemCount(); ++i) {
			  	QTreeWidgetItem *twi = channelList->topLevelItem(i);
				if (twi->text(0) == msg[0]) {
				  	twi->setToolTip(0, msg[1]);
					twi->setText(1, msg[2]);
					return;
				}
			}
			QTreeWidgetItem *twi = new QTreeWidgetItem(QStringList() << msg[0] << msg[2]);
			twi->setTextAlignment(1, Qt::AlignRight);
			twi->setToolTip(0, msg[1]);
			channelList->addTopLevelItem(twi);
			channelList->resizeColumnToContents(0);
			channelList->resizeColumnToContents(1);
			if (msg[3] == "1")
				joinChannel(msg[0]);
			break;
		}
		case eventChatJoinChannel: {
			if (msg.size() != 2)
				break;
			ChannelWidget *w = getChannel(msg[0]);
			if (!w)
				break;
			w->joinEvent(msg[1]);
			break;
		}
		case eventChatListPlayers: {
			if (msg.size() != 2)
				break;
			ChannelWidget *w = getChannel(msg[0]);
			if (!w)
				break;
			w->listPlayersEvent(msg[1]);
			break;
		}
		case eventChatLeaveChannel: {
			if (msg.size() != 2)
				break;
			ChannelWidget *w = getChannel(msg[0]);
			if (!w)
				break;
			w->leaveEvent(msg[1]);
			break;
		}
		case eventChatSay: {
			if (msg.size() != 3)
				break;
			ChannelWidget *w = getChannel(msg[0]);
			if (!w)
				break;
			w->sayEvent(msg[1], msg[2]);
			break;
		}
		case eventChatServerMessage: {
		  	if (msg.size() != 2)
			  	break;
			ChannelWidget *w;
			if (msg[0].isEmpty()) {
				w = getChannel("Server");
				if (!w) {
					w = new ChannelWidget(client, "Server", true, true);
					tab->addTab(w, "Server");
				}
			} else
			  	w = getChannel(msg[0]);
			w->serverMessageEvent(msg[1]);
			break;
		}
		default: {
		}
	}
}

void ChatWidget::joinChannel(const QString &channelName)
{
	PendingCommand_ChatJoinChannel *pc = client->chatJoinChannel(channelName);
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(joinFinished(ServerResponse)));
}

void ChatWidget::joinClicked()
{
	QTreeWidgetItem *twi = channelList->currentItem();
	if (!twi)
		return;
	QString channelName = twi->text(0);
	if (getChannel(channelName))
		return;
	
	joinChannel(channelName);
}

void ChatWidget::joinFinished(ServerResponse resp)
{
	if (resp != RespOk)
		return;
	
	PendingCommand_ChatJoinChannel *pc = qobject_cast<PendingCommand_ChatJoinChannel *>(sender());
	QString channelName = pc->getChannelName();
	ChannelWidget *cw = new ChannelWidget(client, channelName);
	tab->addTab(cw, channelName);
}

ChannelWidget *ChatWidget::getChannel(const QString &name)
{
	for (int i = 0; i < tab->count(); ++i) {
		ChannelWidget *cw = qobject_cast<ChannelWidget *>(tab->widget(i));
		if (cw->getName() == name)
			return cw;
	}
	return 0;
}
