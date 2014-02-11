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
#include "tab_supervisor.h"
#include "tab_room.h"
#include "tab_userlists.h"
#include "userlist.h"
#include "abstractclient.h"
#include "chatview.h"
#include "gameselector.h"
#include "settingscache.h"

#include "get_pb_extension.h"
#include "pb/room_commands.pb.h"
#include "pb/serverinfo_room.pb.h"
#include "pb/event_list_games.pb.h"
#include "pb/event_join_room.pb.h"
#include "pb/event_leave_room.pb.h"
#include "pb/event_room_say.pb.h"
#include "pending_command.h"

TabRoom::TabRoom(TabSupervisor *_tabSupervisor, AbstractClient *_client, ServerInfo_User *_ownUser, const ServerInfo_Room &info)
    : Tab(_tabSupervisor), client(_client), roomId(info.room_id()), roomName(QString::fromStdString(info.name())), ownUser(_ownUser)
{
    const int gameTypeListSize = info.gametype_list_size();
    for (int i = 0; i < gameTypeListSize; ++i)
        gameTypes.insert(info.gametype_list(i).game_type_id(), QString::fromStdString(info.gametype_list(i).description()));
    
    QMap<int, GameTypeMap> tempMap;
    tempMap.insert(info.room_id(), gameTypes);
    gameSelector = new GameSelector(client, tabSupervisor, this, QMap<int, QString>(), tempMap);
    userList = new UserList(tabSupervisor, client, UserList::RoomList);
    connect(userList, SIGNAL(openMessageDialog(const QString &, bool)), this, SIGNAL(openMessageDialog(const QString &, bool)));
    
    chatView = new ChatView(tabSupervisor, 0, true);
    connect(chatView, SIGNAL(openMessageDialog(QString, bool)), this, SIGNAL(openMessageDialog(QString, bool)));
    connect(chatView, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
    connect(chatView, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
    sayLabel = new QLabel;
    sayEdit = new QLineEdit;
    sayLabel->setBuddy(sayEdit);
    connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    
    QMenu *chatSettingsMenu = new QMenu(this);
    aIgnoreUnregisteredUsers = chatSettingsMenu->addAction(QString());
    aIgnoreUnregisteredUsers->setCheckable(true);
    connect(aIgnoreUnregisteredUsers, SIGNAL(triggered()), this, SLOT(actIgnoreUnregisteredUsers()));
    connect(settingsCache, SIGNAL(ignoreUnregisteredUsersChanged()), this, SLOT(ignoreUnregisteredUsersChanged()));    
    QToolButton *chatSettingsButton = new QToolButton;
    chatSettingsButton->setIcon(QIcon(":/resources/icon_settings.svg"));
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

    retranslateUi();
    setLayout(hbox);
    
    const int userListSize = info.user_list_size();
    for (int i = 0; i < userListSize; ++i)
        userList->processUserInfo(info.user_list(i), true);
    userList->sortItems();
    
    const int gameListSize = info.game_list_size();
    for (int i = 0; i < gameListSize; ++i)
        gameSelector->processGameInfo(info.game_list(i));
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
    aIgnoreUnregisteredUsers->setText(tr("&Ignore unregistered users in chat"));
}

void TabRoom::closeRequest()
{
    actLeaveRoom();
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
    if (sayEdit->text().isEmpty())
          return;
    
    Command_RoomSay cmd;
    cmd.set_message(sayEdit->text().toStdString());
    
    PendingCommand *pend = prepareRoomCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(sayFinished(const Response &)));
    sendRoomCommand(pend);
    sayEdit->clear();
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

void TabRoom::actIgnoreUnregisteredUsers()
{
    aIgnoreUnregisteredUsers->setChecked(!aIgnoreUnregisteredUsers->isChecked());
    settingsCache->setIgnoreUnregisteredUsers(!settingsCache->getIgnoreUnregisteredUsers());
}

void TabRoom::ignoreUnregisteredUsersChanged()
{
    aIgnoreUnregisteredUsers->setChecked(settingsCache->getIgnoreUnregisteredUsers());
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
}

void TabRoom::processLeaveRoomEvent(const Event_LeaveRoom &event)
{
    userList->deleteUser(QString::fromStdString(event.name()));
}

void TabRoom::processRoomSayEvent(const Event_RoomSay &event)
{
    QString senderName = QString::fromStdString(event.name());
    if (tabSupervisor->getUserListsTab()->getIgnoreList()->getUsers().contains(senderName))
        return;
    UserListTWI *twi = userList->getUsers().value(senderName);
    UserLevelFlags userLevel;
    if (twi) {
        userLevel = UserLevelFlags(twi->getUserInfo().user_level());
        if (settingsCache->getIgnoreUnregisteredUsers() && !userLevel.testFlag(ServerInfo_User::IsRegistered))
            return;
    }
    chatView->appendMessage(QString::fromStdString(event.message()), senderName, userLevel);
    emit userEvent(false);
}

PendingCommand *TabRoom::prepareRoomCommand(const ::google::protobuf::Message &cmd)
{
    return client->prepareRoomCommand(cmd, roomId);
}

void TabRoom::sendRoomCommand(PendingCommand *pend)
{
    client->sendCommand(pend);
}
