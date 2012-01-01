#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include "tab_message.h"
#include "abstractclient.h"
#include "chatview.h"

#include "pending_command.h"
#include "pb/session_commands.pb.h"
#include "pb/event_user_message.pb.h"

TabMessage::TabMessage(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &_ownName, const QString &_userName)
	: Tab(_tabSupervisor), client(_client), userName(_userName), userOnline(true)
{
	chatView = new ChatView(_ownName, true);
	connect(chatView, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
	connect(chatView, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
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

void TabMessage::closeRequest()
{
	actLeave();
}

void TabMessage::sendMessage()
{
	if (sayEdit->text().isEmpty() || !userOnline)
	  	return;
	
	Command_Message cmd;
	cmd.set_user_name(userName.toStdString());
	cmd.set_message(sayEdit->text().toStdString());
	
	PendingCommand *pend = client->prepareSessionCommand(cmd);
	connect(pend, SIGNAL(finished(const Response &)), this, SLOT(messageSent(const Response &)));
	client->sendCommand(pend);
	
	sayEdit->clear();
}

void TabMessage::messageSent(const Response &response)
{
	if (response.response_code() == Response::RespInIgnoreList)
		chatView->appendMessage(QString(), tr("This user is ignoring you."));
}

void TabMessage::actLeave()
{
	deleteLater();
}

void TabMessage::processUserMessageEvent(const Event_UserMessage &event)
{
	chatView->appendMessage(QString::fromStdString(event.sender_name()), QString::fromStdString(event.message()));
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
