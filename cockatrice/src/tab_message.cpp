#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include "tab_message.h"
#include "abstractclient.h"
#include "protocol_items.h"
#include "chatview.h"

TabMessage::TabMessage(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &_ownName, const QString &_userName)
	: Tab(_tabSupervisor), client(_client), userName(_userName), userOnline(true)
{
	chatView = new ChatView(_ownName);
	sayEdit = new QLineEdit;
	connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(chatView);
	vbox->addWidget(sayEdit);
	
	aLeave = new QAction(this);
	connect(aLeave, SIGNAL(triggered()), this, SLOT(actLeave()));

	tabMenu = new QMenu(this);
	tabMenu->addAction(aLeave);

	retranslateUi();
	setLayout(vbox);
}

TabMessage::~TabMessage()
{
	emit talkClosing(this);
}

void TabMessage::retranslateUi()
{
	tabMenu->setTitle(tr("Personal &talk"));
	aLeave->setText(tr("&Leave"));
}

void TabMessage::sendMessage()
{
	if (sayEdit->text().isEmpty() || !userOnline)
	  	return;
	
	client->sendCommand(new Command_Message(userName, sayEdit->text()));
	sayEdit->clear();
}

void TabMessage::actLeave()
{
	deleteLater();
}

void TabMessage::processMessageEvent(Event_Message *event)
{
	chatView->appendMessage(event->getSenderName(), event->getText());
	emit userEvent();
}

void TabMessage::processUserLeft()
{
	chatView->appendMessage(QString(), tr("%1 has left the server.").arg(userName));
	userOnline = false;
}

void TabMessage::processUserJoined()
{
	chatView->appendMessage(QString(), tr("%1 has joined the server.").arg(userName));
	userOnline = true;
}
