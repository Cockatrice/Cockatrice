#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QSplitter>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QCompleter>
#include <QWidget>
#include <QtCore/qdatetime.h>
#include "tab_supervisor.h"
#include "tab_room.h"
#include "tab_userlists.h"
#include "userlist.h"
#include "abstractclient.h"
#include "chatview.h"
#include "gameselector.h"
#include "settingscache.h"
#include "main.h"
#include "lineeditcompleter.h"
#include "get_pb_extension.h"
#include "pb/room_commands.pb.h"
#include "pb/serverinfo_room.pb.h"
#include "pb/event_list_games.pb.h"
#include "pb/event_join_room.pb.h"
#include "pb/event_leave_room.pb.h"
#include "pb/event_room_say.pb.h"
#include "pending_command.h"
#include "dlg_settings.h"


TabRoom::TabRoom(TabSupervisor *_tabSupervisor, AbstractClient *_client, ServerInfo_User *_ownUser, const ServerInfo_Room &info)
    : Tab(_tabSupervisor), client(_client), roomId(info.room_id()), roomName(QString::fromStdString(info.name())), ownUser(_ownUser)
{
    const int gameTypeListSize = info.gametype_list_size();
    for (int i = 0; i < gameTypeListSize; ++i)
        gameTypes.insert(info.gametype_list(i).game_type_id(), QString::fromStdString(info.gametype_list(i).description()));

    QMap<int, GameTypeMap> tempMap;
    tempMap.insert(info.room_id(), gameTypes);
    gameSelector = new GameSelector(client, tabSupervisor, this, QMap<int, QString>(), tempMap, true, true);
    userList = new UserList(tabSupervisor, client, UserList::RoomList);
    connect(userList, SIGNAL(openMessageDialog(const QString &, bool)), this, SIGNAL(openMessageDialog(const QString &, bool)));

    chatView = new ChatView(tabSupervisor, 0, true);
    connect(chatView, SIGNAL(showMentionPopup(QString&)), this, SLOT(actShowMentionPopup(QString&)));
    connect(chatView, SIGNAL(messageClickedSignal()), this, SLOT(focusTab()));
    connect(chatView, SIGNAL(openMessageDialog(QString, bool)), this, SIGNAL(openMessageDialog(QString, bool)));
    connect(chatView, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
    connect(chatView, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
    connect(chatView, SIGNAL(addMentionTag(QString)), this, SLOT(addMentionTag(QString)));
    connect(settingsCache, SIGNAL(chatMentionCompleterChanged()), this, SLOT(actCompleterChanged()));
    sayLabel = new QLabel;
    sayEdit = new LineEditCompleter;
    sayLabel->setBuddy(sayEdit);
    connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));

    QMenu *chatSettingsMenu = new QMenu(this);

    aClearChat = chatSettingsMenu->addAction(QString());
    connect(aClearChat, SIGNAL(triggered()), this, SLOT(actClearChat()));

    chatSettingsMenu->addSeparator();

    aOpenChatSettings = chatSettingsMenu->addAction(QString());
    connect(aOpenChatSettings, SIGNAL(triggered()), this, SLOT(actOpenChatSettings()));

    QToolButton *chatSettingsButton = new QToolButton;
    chatSettingsButton->setIcon(QPixmap("theme:icons/settings"));
    chatSettingsButton->setMenu(chatSettingsMenu);
    chatSettingsButton->setPopupMode(QToolButton::InstantPopup);

    QHBoxLayout *sayHbox = new QHBoxLayout;
    sayHbox->addWidget(sayLabel);
    sayHbox->addWidget(sayEdit);
    sayHbox->addWidget(chatSettingsButton);

    QVBoxLayout *chatVbox = new QVBoxLayout;
    chatVbox->addWidget(chatView);
    chatVbox->addLayout(sayHbox);

    chatGroupBox = new QGroupBox;
    chatGroupBox->setLayout(chatVbox);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(gameSelector);
    splitter->addWidget(chatGroupBox);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(splitter, 3);
    hbox->addWidget(userList, 1);

    aLeaveRoom = new QAction(this);
    connect(aLeaveRoom, SIGNAL(triggered()), this, SLOT(actLeaveRoom()));

    roomMenu = new QMenu(this);
    roomMenu->addAction(aLeaveRoom);
    addTabMenu(roomMenu);

    const int userListSize = info.user_list_size();
    for (int i = 0; i < userListSize; ++i){
        userList->processUserInfo(info.user_list(i), true);
        autocompleteUserList.append("@" + QString::fromStdString(info.user_list(i).name()));
    }
    userList->sortItems();

    const int gameListSize = info.game_list_size();
    for (int i = 0; i < gameListSize; ++i)
        gameSelector->processGameInfo(info.game_list(i));

    completer = new QCompleter(autocompleteUserList, sayEdit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(5);
    completer->setFilterMode(Qt::MatchStartsWith);

    sayEdit->setCompleter(completer);
    actCompleterChanged();
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutchanged()),this,SLOT(refreshShortcuts()));
    refreshShortcuts();

    retranslateUi();

    QWidget * mainWidget = new QWidget(this);
    mainWidget->setLayout(hbox);
    setCentralWidget(mainWidget);
}

TabRoom::~TabRoom()
{
    emit roomClosing(this);
}

void TabRoom::retranslateUi()
{
    gameSelector->retranslateUi();
    chatView->retranslateUi();
    userList->retranslateUi();
    sayLabel->setText(tr("&Say:"));
    chatGroupBox->setTitle(tr("Chat"));
    roomMenu->setTitle(tr("&Room"));
    aLeaveRoom->setText(tr("&Leave room"));
    aClearChat->setText(tr("&Clear chat"));
    aOpenChatSettings->setText(tr("Chat Settings..."));
}

void TabRoom::focusTab() {
    QApplication::setActiveWindow(this);
    tabSupervisor->setCurrentIndex(tabSupervisor->indexOf(this));
    emit maximizeClient();
}

void TabRoom::actShowMentionPopup(QString &sender) {
    if (trayIcon && (tabSupervisor->currentIndex() != tabSupervisor->indexOf(this) || QApplication::activeWindow() == 0
        || QApplication::focusWidget() == 0)) {
        disconnect(trayIcon, SIGNAL(messageClicked()), 0, 0);
        trayIcon->showMessage(sender + tr(" mentioned you."), tr("Click to view"));
        connect(trayIcon, SIGNAL(messageClicked()), chatView, SLOT(actMessageClicked()));
    }
}

void TabRoom::closeRequest()
{
    actLeaveRoom();
}

void TabRoom::tabActivated()
{
    if(!sayEdit->hasFocus())
        sayEdit->setFocus();
}

QString TabRoom::sanitizeHtml(QString dirty) const
{
    return dirty
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;");
}

void TabRoom::sendMessage()
{
    if (sayEdit->text().isEmpty()){
        return;
    }else if (completer->popup()->isVisible()){
        completer->popup()->hide();
        return;
    }else{
        Command_RoomSay cmd;
        cmd.set_message(sayEdit->text().toStdString());

        PendingCommand *pend = prepareRoomCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(sayFinished(const Response &)));
        sendRoomCommand(pend);
        sayEdit->clear();
    }
}

void TabRoom::sayFinished(const Response &response)
{
    if (response.response_code() == Response::RespChatFlood)
        chatView->appendMessage(tr("You are flooding the chat. Please wait a couple of seconds."));
}

void TabRoom::actLeaveRoom()
{
    sendRoomCommand(prepareRoomCommand(Command_LeaveRoom()));
    deleteLater();
}

void TabRoom::actClearChat() {
    chatView->clearChat();
}

void TabRoom::actOpenChatSettings() {
    DlgSettings settings(this);
    settings.setTab(4);
    settings.exec();
}

void TabRoom::actCompleterChanged()
{
    settingsCache->getChatMentionCompleter() ? completer->setCompletionRole(2) : completer->setCompletionRole(1);
}

void TabRoom::processRoomEvent(const RoomEvent &event)
{
    switch (static_cast<RoomEvent::RoomEventType>(getPbExtension(event))) {
        case RoomEvent::LIST_GAMES: processListGamesEvent(event.GetExtension(Event_ListGames::ext)); break;
        case RoomEvent::JOIN_ROOM: processJoinRoomEvent(event.GetExtension(Event_JoinRoom::ext)); break;
        case RoomEvent::LEAVE_ROOM: processLeaveRoomEvent(event.GetExtension(Event_LeaveRoom::ext)); break;
        case RoomEvent::ROOM_SAY: processRoomSayEvent(event.GetExtension(Event_RoomSay::ext)); break;
        default: ;
    }
}

void TabRoom::processListGamesEvent(const Event_ListGames &event)
{
    const int gameListSize = event.game_list_size();
    for (int i = 0; i < gameListSize; ++i)
        gameSelector->processGameInfo(event.game_list(i));
}

void TabRoom::processJoinRoomEvent(const Event_JoinRoom &event)
{
    userList->processUserInfo(event.user_info(), true);
    userList->sortItems();
    if (!autocompleteUserList.contains("@" + QString::fromStdString(event.user_info().name()))){
        autocompleteUserList << "@" + QString::fromStdString(event.user_info().name());
        sayEdit->setCompletionList(autocompleteUserList);
    }    
}

void TabRoom::processLeaveRoomEvent(const Event_LeaveRoom &event)
{
    userList->deleteUser(QString::fromStdString(event.name()));
    autocompleteUserList.removeOne("@" + QString::fromStdString(event.name()));
    sayEdit->setCompletionList(autocompleteUserList);
}

void TabRoom::processRoomSayEvent(const Event_RoomSay &event)
{
    QString senderName = QString::fromStdString(event.name());
    QString message = QString::fromStdString(event.message());

    if (tabSupervisor->getUserListsTab()->getIgnoreList()->getUsers().contains(senderName))
        return;

    UserListTWI *twi = userList->getUsers().value(senderName);
    UserLevelFlags userLevel;
    QString userPrivLevel;
    if (twi) {
        userLevel = UserLevelFlags(twi->getUserInfo().user_level());
        userPrivLevel = QString::fromStdString(twi->getUserInfo().privlevel());
        if (settingsCache->getIgnoreUnregisteredUsers() && !userLevel.testFlag(ServerInfo_User::IsRegistered))
            return;
    }

    if (event.message_type() == Event_RoomSay::ChatHistory && !settingsCache->getRoomHistory())
        return;

    if (event.message_type() == Event_RoomSay::ChatHistory)
        message = "[" + QString(QDateTime::fromMSecsSinceEpoch(event.time_of()).toLocalTime().toString("d MMM yyyy HH:mm:ss")) + "] " + message;


    chatView->appendMessage(message, event.message_type(), senderName, userLevel, userPrivLevel, true);
    emit userEvent(false);
}

void TabRoom::refreshShortcuts()
{
    aClearChat->setShortcuts(settingsCache->shortcuts().getShortcut("tab_room/aClearChat"));
}

void TabRoom::addMentionTag(QString mentionTag) {
    sayEdit->insert(mentionTag + " ");
    sayEdit->setFocus();
}

PendingCommand *TabRoom::prepareRoomCommand(const ::google::protobuf::Message &cmd)
{
    return client->prepareRoomCommand(cmd, roomId);
}

void TabRoom::sendRoomCommand(PendingCommand *pend)
{
    client->sendCommand(pend);
}