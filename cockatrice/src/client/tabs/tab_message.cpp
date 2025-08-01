#include "tab_message.h"

#include "../../deck/custom_line_edit.h"
#include "../../main.h"
#include "../../server/chat_view/chat_view.h"
#include "../../server/pending_command.h"
#include "../../server/user/user_list_manager.h"
#include "../../settings/cache_settings.h"
#include "../game_logic/abstract_client.h"
#include "../sound_engine.h"
#include "pb/event_user_message.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/session_commands.pb.h"
#include "trice_limits.h"

#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QVBoxLayout>

TabMessage::TabMessage(TabSupervisor *_tabSupervisor,
                       AbstractClient *_client,
                       const ServerInfo_User &_ownUserInfo,
                       const ServerInfo_User &_otherUserInfo)
    : Tab(_tabSupervisor), client(_client), ownUserInfo(new ServerInfo_User(_ownUserInfo)),
      otherUserInfo(new ServerInfo_User(_otherUserInfo)), userOnline(true)
{
    chatView = new ChatView(tabSupervisor, 0, true);
    connect(chatView, &ChatView::showCardInfoPopup, this, &TabMessage::showCardInfoPopup);
    connect(chatView, &ChatView::deleteCardInfoPopup, this, &TabMessage::deleteCardInfoPopup);
    connect(chatView, &ChatView::addMentionTag, this, &TabMessage::addMentionTag);
    sayEdit = new LineEditUnfocusable;
    sayEdit->setMaxLength(MAX_TEXT_LENGTH);
    connect(sayEdit, &LineEditUnfocusable::returnPressed, this, &TabMessage::sendMessage);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(chatView);
    vbox->addWidget(sayEdit);

    aLeave = new QAction(this);
    connect(aLeave, &QAction::triggered, this, &TabMessage::closeRequest);

    messageMenu = new QMenu(this);
    messageMenu->addAction(aLeave);
    addTabMenu(messageMenu);

    retranslateUi();

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(vbox);
    setCentralWidget(mainWidget);
}

TabMessage::~TabMessage()
{
    delete ownUserInfo;
    delete otherUserInfo;
}

void TabMessage::addMentionTag(QString mentionTag)
{
    sayEdit->insert(mentionTag + " ");
    sayEdit->setFocus();
}

void TabMessage::retranslateUi()
{
    messageMenu->setTitle(tr("Private &chat"));
    aLeave->setText(tr("&Leave"));
}

void TabMessage::tabActivated()
{
    if (!sayEdit->hasFocus())
        sayEdit->setFocus();
}

QString TabMessage::getUserName() const
{
    return QString::fromStdString(otherUserInfo->name());
}

QString TabMessage::getTabText() const
{
    return tr("%1 - Private chat").arg(QString::fromStdString(otherUserInfo->name()));
}

void TabMessage::closeEvent(QCloseEvent *event)
{
    emit talkClosing(this);
    event->accept();
}

void TabMessage::sendMessage()
{
    if (sayEdit->text().isEmpty() || !userOnline)
        return;

    Command_Message cmd;
    cmd.set_user_name(otherUserInfo->name());
    cmd.set_message(sayEdit->text().toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabMessage::messageSent);
    client->sendCommand(pend);

    sayEdit->clear();
}

void TabMessage::messageSent(const Response &response)
{
    if (response.response_code() == Response::RespInIgnoreList)
        chatView->appendMessage(tr(
            "This user is ignoring you, they cannot see your messages in main chat and you cannot join their games."));
}

void TabMessage::processUserMessageEvent(const Event_UserMessage &event)
{
    auto userInfo = event.sender_name() == otherUserInfo->name() ? otherUserInfo : ownUserInfo;

    chatView->appendMessage(QString::fromStdString(event.message()), {}, *userInfo, true);
    if (tabSupervisor->currentIndex() != tabSupervisor->indexOf(this))
        soundEngine->playSound("private_message");
    if (SettingsCache::instance().getShowMessagePopup() && shouldShowSystemPopup(event))
        showSystemPopup(event);
    if (QString::fromStdString(event.sender_name()).toLower().simplified() == "servatrice")
        sayEdit->setDisabled(true);

    emit userEvent();
}

bool TabMessage::shouldShowSystemPopup(const Event_UserMessage &event)
{
    return (QApplication::activeWindow() == 0 || QApplication::focusWidget() == 0 ||
            (event.sender_name() == otherUserInfo->name() &&
             tabSupervisor->currentIndex() != tabSupervisor->indexOf(this)));
}

void TabMessage::showSystemPopup(const Event_UserMessage &event)
{
    if (trayIcon) {
        disconnect(trayIcon, &QSystemTrayIcon::messageClicked, 0, 0);
        trayIcon->showMessage(tr("Private message from") + " " + otherUserInfo->name().c_str(),
                              event.message().c_str());
        connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &TabMessage::messageClicked);
    } else {
        qCWarning(TabMessageLog) << "Error: trayIcon is NULL. TabMessage::showSystemPopup failed";
    }
}

void TabMessage::messageClicked()
{
    tabSupervisor->setCurrentIndex(tabSupervisor->indexOf(this));
    activateWindow();
    emit maximizeClient();
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
