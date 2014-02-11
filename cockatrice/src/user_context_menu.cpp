#include <QAction>
#include <QMenu>
#include "user_context_menu.h"
#include "tab_supervisor.h"
#include "tab_userlists.h"
#include "tab_game.h"
#include "userlist.h"
#include "abstractclient.h"
#include "userinfobox.h"
#include "gameselector.h"
#include "pending_command.h"

#include "pb/commands.pb.h"
#include "pb/session_commands.pb.h"
#include "pb/moderator_commands.pb.h"
#include "pb/command_kick_from_game.pb.h"
#include "pb/response_get_games_of_user.pb.h"
#include "pb/response_get_user_info.pb.h"

UserContextMenu::UserContextMenu(const TabSupervisor *_tabSupervisor, QWidget *parent, TabGame *_game)
    : QObject(parent), client(_tabSupervisor->getClient()), tabSupervisor(_tabSupervisor), game(_game)
{
    aUserName = new QAction(QString(), this);
    aUserName->setEnabled(false);
    aDetails = new QAction(QString(), this);
    aChat = new QAction(QString(), this);
    aShowGames = new QAction(QString(), this);
    aAddToBuddyList = new QAction(QString(), this);
    aRemoveFromBuddyList = new QAction(QString(), this);
    aAddToIgnoreList = new QAction(QString(), this);
    aRemoveFromIgnoreList = new QAction(QString(), this);
    aKick = new QAction(QString(), this);
    aBan = new QAction(QString(), this);
    
    retranslateUi();
}

void UserContextMenu::retranslateUi()
{
    aDetails->setText(tr("User &details"));
    aChat->setText(tr("Direct &chat"));
    aShowGames->setText(tr("Show this user's &games"));
    aAddToBuddyList->setText(tr("Add to &buddy list"));
    aRemoveFromBuddyList->setText(tr("Remove from &buddy list"));
    aAddToIgnoreList->setText(tr("Add to &ignore list"));
    aRemoveFromIgnoreList->setText(tr("Remove from &ignore list"));
    aKick->setText(tr("Kick from &game"));
    aBan->setText(tr("Ban from &server"));
}

void UserContextMenu::gamesOfUserReceived(const Response &resp, const CommandContainer &commandContainer)
{
    const Response_GetGamesOfUser &response = resp.GetExtension(Response_GetGamesOfUser::ext);
    const Command_GetGamesOfUser &cmd = commandContainer.session_command(0).GetExtension(Command_GetGamesOfUser::ext);
    
    QMap<int, GameTypeMap> gameTypeMap;
    QMap<int, QString> roomMap;
    const int roomListSize = response.room_list_size();
    for (int i = 0; i < roomListSize; ++i) {
        const ServerInfo_Room &roomInfo = response.room_list(i);
        roomMap.insert(roomInfo.room_id(), QString::fromStdString(roomInfo.name()));
        GameTypeMap tempMap;
        const int gameTypeListSize = roomInfo.gametype_list_size();
        for (int j = 0; j < gameTypeListSize; ++j) {
            const ServerInfo_GameType &gameTypeInfo = roomInfo.gametype_list(j);
            tempMap.insert(gameTypeInfo.game_type_id(), QString::fromStdString(gameTypeInfo.description()));
        }
        gameTypeMap.insert(roomInfo.room_id(), tempMap);
    }
    
    GameSelector *selector = new GameSelector(client, tabSupervisor, 0, roomMap, gameTypeMap);
    const int gameListSize = response.game_list_size();
    for (int i = 0; i < gameListSize; ++i)
        selector->processGameInfo(response.game_list(i));
    
    selector->setWindowTitle(tr("%1's games").arg(QString::fromStdString(cmd.user_name())));
    selector->setAttribute(Qt::WA_DeleteOnClose);
    selector->show();
}

void UserContextMenu::banUser_processUserInfoResponse(const Response &r)
{
    const Response_GetUserInfo &response = r.GetExtension(Response_GetUserInfo::ext);
    
    // The dialog needs to be non-modal in order to not block the event queue of the client.
    BanDialog *dlg = new BanDialog(response.user_info(), static_cast<QWidget *>(parent()));
    connect(dlg, SIGNAL(accepted()), this, SLOT(banUser_dialogFinished()));
    dlg->show();
}

void UserContextMenu::banUser_dialogFinished()
{
    BanDialog *dlg = static_cast<BanDialog *>(sender());
    
    Command_BanFromServer cmd;
    cmd.set_user_name(dlg->getBanName().toStdString());
    cmd.set_address(dlg->getBanIP().toStdString());
    cmd.set_minutes(dlg->getMinutes());
    cmd.set_reason(dlg->getReason().toStdString());
    cmd.set_visible_reason(dlg->getVisibleReason().toStdString());
    
    client->sendCommand(client->prepareModeratorCommand(cmd));
}

void UserContextMenu::showContextMenu(const QPoint &pos, const QString &userName, UserLevelFlags userLevel, int playerId)
{
    aUserName->setText(userName);
    
    QMenu *menu = new QMenu(static_cast<QWidget *>(parent()));
    menu->addAction(aUserName);
    menu->addSeparator();
    menu->addAction(aDetails);
    menu->addAction(aShowGames);
    menu->addAction(aChat);
    if (userLevel.testFlag(ServerInfo_User::IsRegistered) && (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsRegistered)) {
        menu->addSeparator();
        if (tabSupervisor->getUserListsTab()->getBuddyList()->getUsers().contains(userName))
            menu->addAction(aRemoveFromBuddyList);
        else
            menu->addAction(aAddToBuddyList);
        if (tabSupervisor->getUserListsTab()->getIgnoreList()->getUsers().contains(userName))
            menu->addAction(aRemoveFromIgnoreList);
        else
            menu->addAction(aAddToIgnoreList);
    }
    if (game && (game->isHost() || !tabSupervisor->getAdminLocked())) {
        menu->addSeparator();
        menu->addAction(aKick);
    }
    if (!tabSupervisor->getAdminLocked()) {
        menu->addSeparator();
        menu->addAction(aBan);
    }
    bool anotherUser = userName != QString::fromStdString(tabSupervisor->getUserInfo()->name());
    aChat->setEnabled(anotherUser);
    aShowGames->setEnabled(anotherUser);
    aAddToBuddyList->setEnabled(anotherUser);
    aRemoveFromBuddyList->setEnabled(anotherUser);
    aAddToIgnoreList->setEnabled(anotherUser);
    aRemoveFromIgnoreList->setEnabled(anotherUser);
    aKick->setEnabled(anotherUser);
    aBan->setEnabled(anotherUser);
    
    QAction *actionClicked = menu->exec(pos);
    if (actionClicked == aDetails) {
        UserInfoBox *infoWidget = new UserInfoBox(client, true, static_cast<QWidget *>(parent()), Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
        infoWidget->setAttribute(Qt::WA_DeleteOnClose);
        infoWidget->updateInfo(userName);
    } else if (actionClicked == aChat)
        emit openMessageDialog(userName, true);
    else if (actionClicked == aShowGames) {
        Command_GetGamesOfUser cmd;
        cmd.set_user_name(userName.toStdString());
        
        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(gamesOfUserReceived(Response, CommandContainer)));
        
        client->sendCommand(pend);
    } else if (actionClicked == aAddToBuddyList) {
        Command_AddToList cmd;
        cmd.set_list("buddy");
        cmd.set_user_name(userName.toStdString());
        
        client->sendCommand(client->prepareSessionCommand(cmd));
    } else if (actionClicked == aRemoveFromBuddyList) {
        Command_RemoveFromList cmd;
        cmd.set_list("buddy");
        cmd.set_user_name(userName.toStdString());
        
        client->sendCommand(client->prepareSessionCommand(cmd));
    } else if (actionClicked == aAddToIgnoreList) {
        Command_AddToList cmd;
        cmd.set_list("ignore");
        cmd.set_user_name(userName.toStdString());
        
        client->sendCommand(client->prepareSessionCommand(cmd));
    } else if (actionClicked == aRemoveFromIgnoreList) {
        Command_RemoveFromList cmd;
        cmd.set_list("ignore");
        cmd.set_user_name(userName.toStdString());
        
        client->sendCommand(client->prepareSessionCommand(cmd));
    } else if (actionClicked == aKick) {
        Command_KickFromGame cmd;
        cmd.set_player_id(playerId);
        game->sendGameCommand(cmd);
    } else if (actionClicked == aBan) {
        Command_GetUserInfo cmd;
        cmd.set_user_name(userName.toStdString());
        
        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(banUser_processUserInfoResponse(Response)));
        
        client->sendCommand(pend);
    }
    
    delete menu;
}
