#include <QtGui>
#include "chatwidget.h"
#include "client.h"

ChannelWidget::ChannelWidget(const QString &_name, QWidget *parent)
	: QWidget(parent), name(_name)
{
	playerList = new QListWidget;
	
	textEdit = new QTextEdit;
	sayEdit = new QLineEdit;
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(textEdit);
	vbox->addWidget(sayEdit);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(vbox, 1);
	hbox->addWidget(playerList);
	
	setLayout(hbox);
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
}

ChatWidget::ChatWidget(Client *_client, QWidget *parent)
	: QWidget(parent), client(_client)
{
	channelList = new QTreeWidget;
	
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
	Q_ASSERT(header != 0);
	header->setText(0, tr("Channel"));
	header->setText(1, tr("Description"));
	header->setText(2, tr("Players"));
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
	hide();
}

void ChatWidget::chatEvent(const ChatEventData &data)
{
	const QStringList &msg = data.getEventData();
	switch (data.getEventType()) {
		case eventChatListChannels: {
			if (msg.size() != 3)
				break;
			channelList->addTopLevelItem(new QTreeWidgetItem(QStringList() << msg[0] << msg[1] << msg[2]));
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
		default: {
		}
	}
}

void ChatWidget::joinClicked()
{
	QTreeWidgetItem *twi = channelList->currentItem();
	if (!twi)
		return;
	QString channelName = twi->text(0);
	if (getChannel(channelName))
		return;
	
	PendingCommand *pc = client->chatJoinChannel(channelName);
	pc->setExtraData(channelName);
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(joinFinished(ServerResponse)));
}

void ChatWidget::joinFinished(ServerResponse resp)
{
	if (resp != RespOk)
		return;
	
	PendingCommand *pc = qobject_cast<PendingCommand *>(sender());
	QString channelName = pc->getExtraData();
	ChannelWidget *cw = new ChannelWidget(channelName);
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
