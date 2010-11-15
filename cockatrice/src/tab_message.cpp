#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include "tab_message.h"
#include "abstractclient.h"
#include "protocol_items.h"

TabMessage::TabMessage(AbstractClient *_client, const QString &_userName)
	: Tab(), client(_client), userName(_userName)
{
	textEdit = new QTextEdit;
	textEdit->setReadOnly(true);
	sayEdit = new QLineEdit;
	connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(textEdit);
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

QString TabMessage::sanitizeHtml(QString dirty) const
{
	return dirty
		.replace("&", "&amp;")
		.replace("<", "&lt;")
		.replace(">", "&gt;");
}

void TabMessage::sendMessage()
{
	if (sayEdit->text().isEmpty())
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
	textEdit->append(QString("<font color=\"") + (event->getSenderName() == userName ? "#0000fe" : "red") + QString("\">%1:</font> %2").arg(sanitizeHtml(event->getSenderName())).arg(sanitizeHtml(event->getText())));
	emit userEvent();
}

void TabMessage::processUserLeft(const QString &name)
{
	if (userName == name) {
		textEdit->append("<font color=\"blue\">" + tr("%1 has left the server.").arg(sanitizeHtml(name)) + "</font>");
		sayEdit->setEnabled(false);
	}
}
