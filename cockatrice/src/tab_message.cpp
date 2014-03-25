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
#include "pb/serverinfo_user.pb.h"

TabMessage::TabMessage(TabSupervisor *_tabSupervisor, AbstractClient *_client, const ServerInfo_User &_ownUserInfo, const ServerInfo_User &_otherUserInfo)
    : Tab(_tabSupervisor), client(_client), ownUserInfo(new ServerInfo_User(_ownUserInfo)), otherUserInfo(new ServerInfo_User(_otherUserInfo)), userOnline(true)
{
    chatView = new ChatView(tabSupervisor, 0, true);
    connect(chatView, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
    connect(chatView, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
    sayEdit = new QLineEdit;
    connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(chatView);
    vbox->addWidget(sayEdit);
    
    aLeave = new QAction(this);
    connect(aLeave, SIGNAL(triggered()), this, SLOT(actLeave()));

    messageMenu = new QMenu(this);
    messageMenu->addAction(aLeave);
    addTabMenu(messageMenu);

    retranslateUi();
    setLayout(vbox);
}

TabMessage::~TabMessage()
{
    emit talkClosing(this);
    delete ownUserInfo;
    delete otherUserInfo;
}

void TabMessage::retranslateUi()
{
    messageMenu->setTitle(tr("Personal &talk"));
    aLeave->setText(tr("&Leave"));
}

QString TabMessage::getUserName() const
{
    return QString::fromStdString(otherUserInfo->name());
}

QString TabMessage::getTabText() const
{
    return tr("Talking to %1").arg(QString::fromStdString(otherUserInfo->name()));
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
    cmd.set_user_name(otherUserInfo->name());
    cmd.set_message(sayEdit->text().toStdString());
    
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(messageSent(const Response &)));
    client->sendCommand(pend);
    
    sayEdit->clear();
}

void TabMessage::messageSent(const Response &response)
{
    if (response.response_code() == Response::RespInIgnoreList)
        chatView->appendMessage(tr("This user is ignoring you."));
}

void TabMessage::actLeave()
{
    deleteLater();
}

void TabMessage::processUserMessageEvent(const Event_UserMessage &event)
{
    const UserLevelFlags userLevel(event.sender_name() == otherUserInfo->name() ? otherUserInfo->user_level() : ownUserInfo->user_level());
    chatView->appendMessage(QString::fromStdString(event.message()), QString::fromStdString(event.sender_name()), userLevel);
    emit userEvent();
}

void TabMessage::processUserLeft()
{
    chatView->appendMessage(tr("%1 has left the server.").arg(QString::fromStdString(otherUserInfo->name())));
    userOnline = false;
}

void TabMessage::processUserJoined(const ServerInfo_User &_userInfo)
{
    chatView->appendMessage(tr("%1 has joined the server.").arg(QString::fromStdString(otherUserInfo->name())));
    userOnline = true;
    *otherUserInfo = _userInfo;
}
