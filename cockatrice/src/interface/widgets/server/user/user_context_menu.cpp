#include "user_context_menu.h"

#include "../../interface/widgets/tabs/tab_account.h"
#include "../../interface/widgets/tabs/tab_game.h"
#include "../../interface/widgets/tabs/tab_supervisor.h"
#include "../chat_view/chat_view.h"
#include "../game_selector.h"
#include "user_info_box.h"
#include "user_list_manager.h"
#include "user_list_proxy.h"
#include "user_list_widget.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSignalMapper>
#include <QtGui>
#include <QtWidgets>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_kick_from_game.pb.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/moderator_commands.pb.h>
#include <libcockatrice/protocol/pb/response_ban_history.pb.h>
#include <libcockatrice/protocol/pb/response_get_admin_notes.pb.h>
#include <libcockatrice/protocol/pb/response_get_games_of_user.pb.h>
#include <libcockatrice/protocol/pb/response_get_user_info.pb.h>
#include <libcockatrice/protocol/pb/response_warn_history.pb.h>
#include <libcockatrice/protocol/pb/response_warn_list.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>
#include <libcockatrice/protocol/pending_command.h>

UserContextMenu::UserContextMenu(TabSupervisor *_tabSupervisor, QWidget *parent, AbstractGame *_game)
    : QObject(parent), client(_tabSupervisor->getClient()), tabSupervisor(_tabSupervisor),
      userListProxy(_tabSupervisor->getUserListManager()), game(_game)
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
    aPromoteToJudge = new QAction(QString(), this);
    aDemoteFromJudge = new QAction(QString(), this);
    aGetAdminNotes = new QAction(QString(), this);

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
    aPromoteToJudge->setText(tr("Promote user to &judge"));
    aDemoteFromJudge->setText(tr("Demote user from judge"));
    aGetAdminNotes->setText(tr("View admin notes"));
}

void UserContextMenu::gamesOfUserReceived(const Response &resp, const CommandContainer &commandContainer)
{
    const Command_GetGamesOfUser &cmd = commandContainer.session_command(0).GetExtension(Command_GetGamesOfUser::ext);
    if (resp.response_code() == Response::RespNameNotFound) {
        QMessageBox::critical(static_cast<QWidget *>(parent()), tr("Error"), tr("This user does not exist."));
        return;
    } else if (resp.response_code() == Response::RespInIgnoreList) {
        QMessageBox::critical(
            static_cast<QWidget *>(parent()), tr("Error"),
            tr("You are being ignored by %1 and can't see their games.").arg(QString::fromStdString(cmd.user_name())));
        return;
    } else if (resp.response_code() != Response::RespOk) {
        QMessageBox::critical(static_cast<QWidget *>(parent()), tr("Error"),
                              tr("Could not get %1's games.").arg(QString::fromStdString(cmd.user_name())));
        return;
    }
    const Response_GetGamesOfUser &response = resp.GetExtension(Response_GetGamesOfUser::ext);

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

    auto *selector = new GameSelector(client, tabSupervisor, nullptr, roomMap, gameTypeMap, false, false);
    selector->setParent(static_cast<QWidget *>(parent()), Qt::Window);
    const int gameListSize = response.game_list_size();
    for (int i = 0; i < gameListSize; ++i) {
        selector->processGameInfo(response.game_list(i));
    }

    selector->setWindowTitle(tr("%1's games").arg(QString::fromStdString(cmd.user_name())));
    selector->setMinimumWidth(800);
    selector->setAttribute(Qt::WA_DeleteOnClose);
    selector->show();
}

void UserContextMenu::banUser_processUserInfoResponse(const Response &r)
{
    const Response_GetUserInfo &response = r.GetExtension(Response_GetUserInfo::ext);

    // The dialog needs to be non-modal in order to not block the event queue of the client.
    auto *dlg = new BanDialog(response.user_info(), static_cast<QWidget *>(parent()));
    connect(dlg, &QDialog::accepted, this, &UserContextMenu::banUser_dialogFinished);
    dlg->show();
}

void UserContextMenu::warnUser_processGetWarningsListResponse(const Response &r)
{
    const Response_WarnList &response = r.GetExtension(Response_WarnList::ext);

    QString user = QString::fromStdString(response.user_name()).simplified();
    QString clientid = QString::fromStdString(response.user_clientid()).simplified();

    // The dialog needs to be non-modal in order to not block the event queue of the client.
    auto *dlg = new WarningDialog(user, clientid, static_cast<QWidget *>(parent()));
    connect(dlg, &QDialog::accepted, this, &UserContextMenu::warnUser_dialogFinished);

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
    connect(pend, &PendingCommand::finished, this, &UserContextMenu::warnUser_processGetWarningsListResponse);
    client->sendCommand(pend);
}

void UserContextMenu::banUserHistory_processResponse(const Response &resp)
{
    const Response_BanHistory &response = resp.GetExtension(Response_BanHistory::ext);
    if (resp.response_code() == Response::RespOk) {

        if (response.ban_list_size() > 0) {
            auto *table = new QTableWidget();
            table->setWindowTitle(tr("Ban History"));
            table->setRowCount(response.ban_list_size());
            table->setColumnCount(5);
            table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            table->setHorizontalHeaderLabels(
                QString(tr("Ban Time;Moderator;Ban Length;Ban Reason;Visible Reason")).split(";"));

            ServerInfo_Ban ban;
            for (int i = 0; i < response.ban_list_size(); ++i) {
                ban = response.ban_list(i);
                table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(ban.ban_time())));
                table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(ban.admin_name())));
                table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(ban.ban_length())));
                table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(ban.ban_reason())));
                table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(ban.visible_reason())));
            }

            table->resizeColumnsToContents();
            table->setMinimumSize(table->horizontalHeader()->length() + (table->columnCount() * 5),
                                  table->verticalHeader()->length() + (table->rowCount() * 3));
            table->show();
        } else
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Ban History"),
                                     tr("User has never been banned."));

    } else
        QMessageBox::critical(static_cast<QWidget *>(parent()), tr("Ban History"),
                              tr("Failed to collect ban information."));
}

void UserContextMenu::warnUserHistory_processResponse(const Response &resp)
{
    const Response_WarnHistory &response = resp.GetExtension(Response_WarnHistory::ext);
    if (resp.response_code() == Response::RespOk) {

        if (response.warn_list_size() > 0) {
            auto *table = new QTableWidget();
            table->setWindowTitle(tr("Warning History"));
            table->setRowCount(response.warn_list_size());
            table->setColumnCount(4);
            table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            table->setHorizontalHeaderLabels(QString(tr("Warning Time;Moderator;User Name;Reason")).split(";"));

            ServerInfo_Warning warn;
            for (int i = 0; i < response.warn_list_size(); ++i) {
                warn = response.warn_list(i);
                table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(warn.time_of())));
                table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(warn.admin_name())));
                table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(warn.user_name())));
                table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(warn.reason())));
            }

            table->resizeColumnsToContents();
            table->setMinimumSize(table->horizontalHeader()->length() + (table->columnCount() * 5),
                                  table->verticalHeader()->length() + (table->rowCount() * 3));
            table->show();
        } else
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Warning History"),
                                     tr("User has never been warned."));

    } else
        QMessageBox::critical(static_cast<QWidget *>(parent()), tr("Warning History"),
                              tr("Failed to collect warning information."));
}

void UserContextMenu::getAdminNotes_processResponse(const Response &resp)
{
    const Response_GetAdminNotes &response = resp.GetExtension(Response_GetAdminNotes::ext);

    if (resp.response_code() != Response::RespOk) {
        QMessageBox::information(static_cast<QWidget *>(parent()), tr("Failed"), tr("Failed to get admin notes."));
        return;
    }

    auto *dlg = new AdminNotesDialog(QString::fromStdString(response.user_name()),
                                     QString::fromStdString(response.notes()), static_cast<QWidget *>(parent()));
    connect(dlg, &AdminNotesDialog::accepted, this, &UserContextMenu::updateAdminNotes_dialogFinished);
    dlg->show();
}

void UserContextMenu::adjustMod_processUserResponse(const Response &resp, const CommandContainer &commandContainer)
{

    const Command_AdjustMod &cmd = commandContainer.admin_command(0).GetExtension(Command_AdjustMod::ext);

    if (resp.response_code() == Response::RespOk) {
        if (cmd.should_be_mod() || cmd.should_be_judge()) {
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Success"),
                                     tr("Successfully promoted user."));
        } else {
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Success"), tr("Successfully demoted user."));
        }

    } else {
        if (cmd.should_be_mod() || cmd.should_be_judge()) {
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Failed"), tr("Failed to promote user."));
        } else {
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Failed"), tr("Failed to demote user."));
        }
    }
}

void UserContextMenu::banUser_dialogFinished()
{
    auto *dlg = static_cast<BanDialog *>(sender());

    Command_BanFromServer cmd;
    cmd.set_user_name(dlg->getBanName().toStdString());
    cmd.set_address(dlg->getBanIP().toStdString());
    cmd.set_minutes(dlg->getMinutes());
    cmd.set_reason(dlg->getReason().toStdString());
    cmd.set_visible_reason(dlg->getVisibleReason().toStdString());
    cmd.set_clientid(dlg->getBanId().toStdString());
    int removeAmount = dlg->getDeleteMessages();
    if (removeAmount != 0) {
        cmd.set_remove_messages(removeAmount);
    }

    client->sendCommand(client->prepareModeratorCommand(cmd));
}

void UserContextMenu::warnUser_dialogFinished()
{
    auto *dlg = static_cast<WarningDialog *>(sender());

    if (dlg->getName().isEmpty() || userListProxy->getOwnUsername().simplified().isEmpty())
        return;

    Command_WarnUser cmd;
    cmd.set_user_name(dlg->getName().toStdString());
    cmd.set_reason(dlg->getReason().toStdString());
    cmd.set_clientid(dlg->getWarnID().toStdString());
    int removeAmount = dlg->getDeleteMessages();
    if (removeAmount != 0) {
        cmd.set_remove_messages(removeAmount);
    }

    client->sendCommand(client->prepareModeratorCommand(cmd));
}

void UserContextMenu::updateAdminNotes_dialogFinished()
{
    auto *dlg = static_cast<AdminNotesDialog *>(sender());

    Command_UpdateAdminNotes cmd;
    cmd.set_user_name(dlg->getName().toStdString());
    cmd.set_notes(dlg->getNotes().toStdString());

    client->sendCommand(client->prepareModeratorCommand(cmd));
}

void UserContextMenu::showContextMenu(const QPoint &pos,
                                      const QString &userName,
                                      UserLevelFlags userLevel,
                                      bool online,
                                      int playerId)
{
    showContextMenu(pos, userName, userLevel, online, playerId, QString(), nullptr);
}

void UserContextMenu::showContextMenu(const QPoint &pos,
                                      const QString &userName,
                                      UserLevelFlags userLevel,
                                      ChatView *chatView)
{
    showContextMenu(pos, userName, userLevel, true, -1, QString(), chatView);
}

void UserContextMenu::showContextMenu(const QPoint &pos,
                                      const QString &userName,
                                      UserLevelFlags userLevel,
                                      bool online,
                                      int playerId,
                                      const QString &deckHash,
                                      ChatView *chatView)
{
    QAction *aCopyToClipBoard = nullptr, *aRemoveMessages = nullptr;
    aUserName->setText(userName);

    auto *menu = new QMenu(static_cast<QWidget *>(parent()));
    menu->addAction(aUserName);
    menu->addSeparator();
    if (!deckHash.isEmpty()) {
        aCopyToClipBoard = new QAction(tr("Copy hash to clipboard"), this);
        menu->addAction(aCopyToClipBoard);
    }
    menu->addAction(aDetails);
    menu->addAction(aShowGames);
    menu->addAction(aChat);
    if (userLevel.testFlag(ServerInfo_User::IsRegistered) && userListProxy->isOwnUserRegistered()) {
        menu->addSeparator();
        if (userListProxy->isUserBuddy(userName)) {
            menu->addAction(aRemoveFromBuddyList);
        } else {
            menu->addAction(aAddToBuddyList);
        }
        if (userListProxy->isUserIgnored(userName)) {
            menu->addAction(aRemoveFromIgnoreList);
        } else {
            menu->addAction(aAddToIgnoreList);
        }
    }
    if (chatView != nullptr) {
        aRemoveMessages = new QAction(tr("Remove this user's messages"), this);
        menu->addAction(aRemoveMessages);
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
        menu->addAction(aGetAdminNotes);

        menu->addSeparator();
        if (userLevel.testFlag(ServerInfo_User::IsModerator) &&
            (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsAdmin)) {
            menu->addAction(aDemoteFromMod);

        } else if (userLevel.testFlag(ServerInfo_User::IsRegistered) &&
                   (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsAdmin)) {
            menu->addAction(aPromoteToMod);
        }

        if (userLevel.testFlag(ServerInfo_User::IsJudge) &&
            (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsAdmin)) {
            menu->addAction(aDemoteFromJudge);

        } else if (userLevel.testFlag(ServerInfo_User::IsRegistered) &&
                   (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsAdmin)) {
            menu->addAction(aPromoteToJudge);
        }
    }
    bool anotherUser = userName != userListProxy->getOwnUsername();
    aDetails->setEnabled(true);
    aChat->setEnabled(anotherUser && online);
    aShowGames->setEnabled(online);
    aAddToBuddyList->setEnabled(anotherUser);
    aRemoveFromBuddyList->setEnabled(anotherUser);
    aAddToIgnoreList->setEnabled(anotherUser);
    aRemoveFromIgnoreList->setEnabled(anotherUser);
    aKick->setEnabled(anotherUser);
    aWarnUser->setEnabled(anotherUser);
    aWarnHistory->setEnabled(anotherUser);
    aBan->setEnabled(anotherUser);
    aBanHistory->setEnabled(anotherUser);
    aGetAdminNotes->setEnabled(anotherUser);
    aPromoteToMod->setEnabled(anotherUser);
    aDemoteFromMod->setEnabled(anotherUser);

    QAction *actionClicked = menu->exec(pos);
    if (actionClicked == nullptr) {
    } else if (actionClicked == aDetails) {
        auto *infoWidget =
            new UserInfoBox(client, false, static_cast<QWidget *>(parent()),
                            Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
        infoWidget->setAttribute(Qt::WA_DeleteOnClose);
        infoWidget->updateInfo(userName);
    } else if (actionClicked == aChat) {
        emit openMessageDialog(userName, true);
    } else if (actionClicked == aShowGames) {
        Command_GetGamesOfUser cmd;
        cmd.set_user_name(userName.toStdString());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &UserContextMenu::gamesOfUserReceived);

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

        game->getGameEventHandler()->sendGameCommand(cmd);
    } else if (actionClicked == aBan) {
        Command_GetUserInfo cmd;
        cmd.set_user_name(userName.toStdString());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &UserContextMenu::banUser_processUserInfoResponse);
        client->sendCommand(pend);
    } else if (actionClicked == aPromoteToMod || actionClicked == aDemoteFromMod) {
        Command_AdjustMod cmd;
        cmd.set_user_name(userName.toStdString());
        cmd.set_should_be_mod(actionClicked == aPromoteToMod);

        PendingCommand *pend = client->prepareAdminCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &UserContextMenu::adjustMod_processUserResponse);
        client->sendCommand(pend);
    } else if (actionClicked == aPromoteToJudge || actionClicked == aDemoteFromJudge) {
        Command_AdjustMod cmd;
        cmd.set_user_name(userName.toStdString());
        cmd.set_should_be_judge(actionClicked == aPromoteToJudge);

        PendingCommand *pend = client->prepareAdminCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &UserContextMenu::adjustMod_processUserResponse);
        client->sendCommand(pend);
    } else if (actionClicked == aBanHistory) {
        Command_GetBanHistory cmd;
        cmd.set_user_name(userName.toStdString());
        PendingCommand *pend = client->prepareModeratorCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &UserContextMenu::banUserHistory_processResponse);
        client->sendCommand(pend);
    } else if (actionClicked == aWarnUser) {
        Command_GetUserInfo cmd;
        cmd.set_user_name(userName.toStdString());
        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &UserContextMenu::warnUser_processUserInfoResponse);
        client->sendCommand(pend);
    } else if (actionClicked == aWarnHistory) {
        Command_GetWarnHistory cmd;
        cmd.set_user_name(userName.toStdString());
        PendingCommand *pend = client->prepareModeratorCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &UserContextMenu::warnUserHistory_processResponse);
        client->sendCommand(pend);
    } else if (actionClicked == aGetAdminNotes) {
        Command_GetAdminNotes cmd;
        cmd.set_user_name(userName.toStdString());
        auto *pend = client->prepareModeratorCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &UserContextMenu::getAdminNotes_processResponse);
        client->sendCommand(pend);

    } else if (actionClicked == aCopyToClipBoard) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(deckHash);
    } else if (actionClicked == aRemoveMessages) {
        chatView->redactMessages(userName, -1);
    }

    delete menu;
}
