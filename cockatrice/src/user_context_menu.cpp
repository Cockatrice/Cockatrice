#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include "user_context_menu.h"
#include "tab_supervisor.h"
#include "tab_userlists.h"
#include "tab_game.h"
#include "userlist.h"
#include "abstractclient.h"
#include "userinfobox.h"
#include "gameselector.h"
#include "pending_command.h"

#include <QtGui>
#include <QtWidgets>
#include <QSignalMapper>

#include "pb/commands.pb.h"
#include "pb/session_commands.pb.h"
#include "pb/moderator_commands.pb.h"
#include "pb/command_kick_from_game.pb.h"
#include "pb/response_get_games_of_user.pb.h"
#include "pb/response_get_user_info.pb.h"
#include "pb/response_ban_history.pb.h"
#include "pb/response_warn_history.pb.h"
#include "pb/response_warn_list.pb.h"

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
    aWarnUser = new QAction(QString(), this);
    aWarnHistory = new QAction(QString(), this);
    aBan = new QAction(QString(), this);
    aBanHistory = new QAction(QString(), this);
    aPromoteToMod = new QAction(QString(), this);
    aDemoteFromMod = new QAction(QString(), this);

    retranslateUi();
}

void UserContextMenu::retranslateUi()
{
    aDetails->setText(tr("User &details"));
    aChat->setText(tr("Private &chat"));
    aShowGames->setText(tr("Show this user's &games"));
    aAddToBuddyList->setText(tr("Add to &buddy list"));
    aRemoveFromBuddyList->setText(tr("Remove from &buddy list"));
    aAddToIgnoreList->setText(tr("Add to &ignore list"));
    aRemoveFromIgnoreList->setText(tr("Remove from &ignore list"));
    aKick->setText(tr("Kick from &game"));
    aWarnUser->setText(tr("Warn user"));
    aWarnHistory->setText(tr("View user's war&n history"));
    aBan->setText(tr("Ban from &server"));
    aBanHistory->setText(tr("View user's &ban history"));
    aPromoteToMod->setText(tr("&Promote user to moderator"));
    aDemoteFromMod->setText(tr("Dem&ote user from moderator"));
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

    GameSelector *selector = new GameSelector(client, tabSupervisor, 0, roomMap, gameTypeMap, false, false);
    const int gameListSize = response.game_list_size();
    for (int i = 0; i < gameListSize; ++i)
        selector->processGameInfo(response.game_list(i));

    selector->setWindowTitle(tr("%1's games").arg(QString::fromStdString(cmd.user_name())));
    selector->setMinimumWidth(800);
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

void UserContextMenu::warnUser_processGetWarningsListResponse(const Response &r)
{
    const Response_WarnList &response = r.GetExtension(Response_WarnList::ext);

    QString user = QString::fromStdString(response.user_name()).simplified();
    QString clientid = QString::fromStdString(response.user_clientid()).simplified();

    // The dialog needs to be non-modal in order to not block the event queue of the client.
    WarningDialog *dlg = new WarningDialog(user, clientid, static_cast<QWidget *>(parent()));
    connect(dlg, SIGNAL(accepted()), this, SLOT(warnUser_dialogFinished()));

    if (response.warning_size() > 0) {
        for (int i = 0; i < response.warning_size(); ++i) {
            dlg->addWarningOption(QString::fromStdString(response.warning(i)).simplified());
        }
    }
    dlg->show();
}

void UserContextMenu::warnUser_processUserInfoResponse(const Response &resp)
{
    const Response_GetUserInfo &response = resp.GetExtension(Response_GetUserInfo::ext);
    ServerInfo_User userInfo = response.user_info();

    Command_GetWarnList cmd;
    cmd.set_user_name(userInfo.name());
    cmd.set_user_clientid(userInfo.clientid());
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(warnUser_processGetWarningsListResponse(Response)));
    client->sendCommand(pend);

}

void UserContextMenu::banUserHistory_processResponse(const Response &resp) {
    const Response_BanHistory &response = resp.GetExtension(Response_BanHistory::ext);
    if (resp.response_code() == Response::RespOk) {

        if (response.ban_list_size() > 0) {
            QTableWidget *table = new QTableWidget();
            table->setWindowTitle(tr("Ban History"));
            table->setRowCount(response.ban_list_size());
            table->setColumnCount(5);
            table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            table->setHorizontalHeaderLabels(
                    QString(tr("Ban Time;Moderator;Ban Length;Ban Reason;Visible Reason")).split(";"));

            ServerInfo_Ban ban; for (int i = 0; i < response.ban_list_size(); ++i) {
                ban = response.ban_list(i);
                table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(ban.ban_time())));
                table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(ban.admin_name())));
                table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(ban.ban_length())));
                table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(ban.ban_reason())));
                table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(ban.visible_reason())));
            }

            table->resizeColumnsToContents();
            table->setMinimumSize(table->horizontalHeader()->length() + (table->columnCount() * 5), table->verticalHeader()->length() + (table->rowCount() * 3));
            table->show();
        } else
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Ban History"), tr("User has never been banned."));

    } else
        QMessageBox::critical(static_cast<QWidget *>(parent()), tr("Ban History"), tr("Failed to collecting ban information."));
}

void UserContextMenu::warnUserHistory_processResponse(const Response &resp) {
    const Response_WarnHistory &response = resp.GetExtension(Response_WarnHistory::ext);
    if (resp.response_code() == Response::RespOk) {

        if (response.warn_list_size() > 0) {
            QTableWidget *table = new QTableWidget();
            table->setWindowTitle(tr("Warning History"));
            table->setRowCount(response.warn_list_size());
            table->setColumnCount(4);
            table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            table->setHorizontalHeaderLabels(
                    QString(tr("Warning Time;Moderator;User Name;Reason")).split(";"));

            ServerInfo_Warning warn; for (int i = 0; i < response.warn_list_size(); ++i) {
                warn = response.warn_list(i);
                table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(warn.time_of())));
                table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(warn.admin_name())));
                table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(warn.user_name())));
                table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(warn.reason())));
            }

            table->resizeColumnsToContents();
            table->setMinimumSize(table->horizontalHeader()->length() + (table->columnCount() * 5), table->verticalHeader()->length() + (table->rowCount() * 3));
            table->show();
        } else
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Warning History"), tr("User has never been warned."));

    } else
        QMessageBox::critical(static_cast<QWidget *>(parent()), tr("Warning History"), tr("Failed to collecting warning information."));
}

void UserContextMenu::adjustMod_processUserResponse(const Response &resp, const CommandContainer &commandContainer)
{

    const Command_AdjustMod &cmd = commandContainer.admin_command(0).GetExtension(Command_AdjustMod::ext);

    if (resp.response_code() == Response::RespOk) {
        if (cmd.should_be_mod()) {
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Success"), tr("Successfully promoted user."));
        } else {
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Success"), tr("Successfully demoted user."));
        }

    } else {
        if (cmd.should_be_mod()) {
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Failed"), tr("Failed to promote user."));
        } else {
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Failed"), tr("Failed to demote user."));
        }
    }
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
    cmd.set_clientid(dlg->getBanId().toStdString());

    client->sendCommand(client->prepareModeratorCommand(cmd));
}

void UserContextMenu::warnUser_dialogFinished()
{
    WarningDialog *dlg = static_cast<WarningDialog *>(sender());

    if (dlg->getName().isEmpty() || QString::fromStdString(tabSupervisor->getUserInfo()->name()).simplified().isEmpty())
        return;

    Command_WarnUser cmd;
    cmd.set_user_name(dlg->getName().toStdString());
    cmd.set_reason(dlg->getReason().toStdString());
    cmd.set_clientid(dlg->getWarnID().toStdString());

    client->sendCommand(client->prepareModeratorCommand(cmd));

}

void UserContextMenu::showContextMenu(const QPoint &pos, const QString &userName, UserLevelFlags userLevel, bool online, int playerId)
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
        menu->addAction(aWarnUser);
        menu->addAction(aWarnHistory);
        menu->addSeparator();
        menu->addAction(aBan);
        menu->addAction(aBanHistory);

        menu->addSeparator();
        if (userLevel.testFlag(ServerInfo_User::IsModerator) && (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsAdmin)) {
            menu->addAction(aDemoteFromMod);

        } else if (userLevel.testFlag(ServerInfo_User::IsRegistered) && (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsAdmin)) {
            menu->addAction(aPromoteToMod);
        }
    }
    bool anotherUser = userName != QString::fromStdString(tabSupervisor->getUserInfo()->name());
    aDetails->setEnabled(true);
    aChat->setEnabled(anotherUser && online);
    aShowGames->setEnabled(anotherUser);
    aAddToBuddyList->setEnabled(anotherUser);
    aRemoveFromBuddyList->setEnabled(anotherUser);
    aAddToIgnoreList->setEnabled(anotherUser);
    aRemoveFromIgnoreList->setEnabled(anotherUser);
    aKick->setEnabled(anotherUser);
    aWarnUser->setEnabled(anotherUser);
    aWarnHistory->setEnabled(anotherUser);
    aBan->setEnabled(anotherUser);
    aBanHistory->setEnabled(anotherUser);
    aPromoteToMod->setEnabled(anotherUser);
    aDemoteFromMod->setEnabled(anotherUser);

    QAction *actionClicked = menu->exec(pos);
    if (actionClicked == aDetails) {
        UserInfoBox *infoWidget = new UserInfoBox(client, false, static_cast<QWidget *>(parent()), Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
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
    } else if (actionClicked == aPromoteToMod || actionClicked == aDemoteFromMod) {
        Command_AdjustMod cmd;
        cmd.set_user_name(userName.toStdString());
        cmd.set_should_be_mod(actionClicked == aPromoteToMod);

        PendingCommand *pend = client->prepareAdminCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(adjustMod_processUserResponse(Response, CommandContainer)));
        client->sendCommand(pend);
    } else if (actionClicked == aBanHistory) {
        Command_GetBanHistory cmd;
        cmd.set_user_name(userName.toStdString());
        PendingCommand *pend = client->prepareModeratorCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(banUserHistory_processResponse(Response)));
        client->sendCommand(pend);
    } else if (actionClicked == aWarnUser) {
        Command_GetUserInfo cmd;
        cmd.set_user_name(userName.toStdString());
        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(warnUser_processUserInfoResponse(Response)));
        client->sendCommand(pend);
    } else if (actionClicked == aWarnHistory) {
        Command_GetWarnHistory cmd;
        cmd.set_user_name(userName.toStdString());
        PendingCommand *pend = client->prepareModeratorCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(warnUserHistory_processResponse(Response)));
        client->sendCommand(pend);
    }

    delete menu;
}
