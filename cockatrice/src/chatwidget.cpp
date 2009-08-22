#include <QtGui>
#include "chatwidget.h"
#include "client.h"

ChannelWidget::ChannelWidget(QWidget *parent)
	: QWidget(parent)
{
	playerList = new QListWidget;
	
	textEdit = new QTextEdit;
	sayEdit = new QLineEdit;
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(textEdit);
	vbox->addWidget(sayEdit);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(vbox);
	hbox->addWidget(playerList);
	
	setLayout(hbox);
}

ChatWidget::ChatWidget(Client *_client, QWidget *parent)
	: QWidget(parent), client(_client)
{
	channelList = new QTreeWidget;
	tab = new QTabWidget;
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(channelList);
	hbox->addWidget(tab, 1);
	
	retranslateUi();
	setLayout(hbox);
}

void ChatWidget::retranslateUi()
{
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
			break;
		}
		case eventChatListPlayers: {
			break;
		}
		case eventChatLeaveChannel: {
			break;
		}
		case eventChatSay: {
			break;
		}
		default: {
		}
	}
}
