/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "serversocketinterface.h"

#include "decklist.h"
#include "main.h"
#include "pb/command_deck_del.pb.h"
#include "pb/command_deck_del_dir.pb.h"
#include "pb/command_deck_download.pb.h"
#include "pb/command_deck_list.pb.h"
#include "pb/command_deck_new_dir.pb.h"
#include "pb/command_deck_upload.pb.h"
#include "pb/command_replay_delete_match.pb.h"
#include "pb/command_replay_download.pb.h"
#include "pb/command_replay_list.pb.h"
#include "pb/command_replay_modify_match.pb.h"
#include "pb/commands.pb.h"
#include "pb/event_add_to_list.pb.h"
#include "pb/event_connection_closed.pb.h"
#include "pb/event_notify_user.pb.h"
#include "pb/event_remove_from_list.pb.h"
#include "pb/event_server_identification.pb.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_user_message.pb.h"
#include "pb/response_ban_history.pb.h"
#include "pb/response_deck_download.pb.h"
#include "pb/response_deck_list.pb.h"
#include "pb/response_deck_upload.pb.h"
#include "pb/response_forgotpasswordrequest.pb.h"
#include "pb/response_password_salt.pb.h"
#include "pb/response_register.pb.h"
#include "pb/response_replay_download.pb.h"
#include "pb/response_replay_list.pb.h"
#include "pb/response_viewlog_history.pb.h"
#include "pb/response_warn_history.pb.h"
#include "pb/response_warn_list.pb.h"
#include "pb/serverinfo_ban.pb.h"
#include "pb/serverinfo_chat_message.pb.h"
#include "pb/serverinfo_deckstorage.pb.h"
#include "pb/serverinfo_replay.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "servatrice.h"
#include "servatrice_database_interface.h"
#include "server_logger.h"
#include "server_player.h"
#include "server_response_containers.h"
#include "server_room.h"
#include "settingscache.h"
#include "stringsizes.h"
#include "version_string.h"

#include <QDateTime>
#include <QDebug>
#include <QHostAddress>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <iostream>
#include <string>

static const int protocolVersion = 14;

AbstractServerSocketInterface::AbstractServerSocketInterface(Servatrice *_server,
                                                             Servatrice_DatabaseInterface *_databaseInterface,
                                                             QObject *parent)
    : Server_ProtocolHandler(_server, _databaseInterface, parent), servatrice(_server),
      sqlInterface(reinterpret_cast<Servatrice_DatabaseInterface *>(databaseInterface))
{
    // Never call flushOutputQueue directly from outputQueueChanged. In case of a socket error,
    // it could lead to this object being destroyed while another function is still on the call stack. -> mutex
    // deadlocks etc.
    connect(this, SIGNAL(outputQueueChanged()), this, SLOT(flushOutputQueue()), Qt::QueuedConnection);
}

bool AbstractServerSocketInterface::initSession()
{
    Event_ServerIdentification identEvent;
    identEvent.set_server_name(servatrice->getServerName().toStdString());
    identEvent.set_server_version(VERSION_STRING);
    identEvent.set_protocol_version(protocolVersion);
    if (servatrice->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
        identEvent.set_server_options(Event_ServerIdentification::SupportsPasswordHash);
    }
    SessionEvent *identSe = prepareSessionEvent(identEvent);
    sendProtocolItem(*identSe);
    delete identSe;

    // allow unlimited number of connections from the trusted sources
    QString trustedSources = settingsCache->value("security/trusted_sources", "127.0.0.1,::1").toString();
    if (trustedSources.contains(getAddress(), Qt::CaseInsensitive))
        return true;

    int maxUsers = servatrice->getMaxUsersPerAddress();
    if ((maxUsers > 0) && (servatrice->getUsersWithAddress(getPeerAddress()) > maxUsers)) {
        Event_ConnectionClosed event;
        event.set_reason(Event_ConnectionClosed::TOO_MANY_CONNECTIONS);
        SessionEvent *se = prepareSessionEvent(event);
        sendProtocolItem(*se);
        delete se;
        return false;
    }

    return true;
}

void AbstractServerSocketInterface::catchSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket error:" << socketError;

    prepareDestroy();
}

void AbstractServerSocketInterface::catchSocketDisconnected()
{
    prepareDestroy();
}

void AbstractServerSocketInterface::transmitProtocolItem(const ServerMessage &item)
{
    outputQueueMutex.lock();
    outputQueue.append(item);
    outputQueueMutex.unlock();

    emit outputQueueChanged();
}

void AbstractServerSocketInterface::logDebugMessage(const QString &message)
{
    logger->logMessage(message, this);
}

Response::ResponseCode AbstractServerSocketInterface::processExtendedSessionCommand(int cmdType,
                                                                                    const SessionCommand &cmd,
                                                                                    ResponseContainer &rc)
{
    switch ((SessionCommand::SessionCommandType)cmdType) {
        case SessionCommand::ADD_TO_LIST:
            return cmdAddToList(cmd.GetExtension(Command_AddToList::ext), rc);
        case SessionCommand::REMOVE_FROM_LIST:
            return cmdRemoveFromList(cmd.GetExtension(Command_RemoveFromList::ext), rc);
        case SessionCommand::DECK_LIST:
            return cmdDeckList(cmd.GetExtension(Command_DeckList::ext), rc);
        case SessionCommand::DECK_NEW_DIR:
            return cmdDeckNewDir(cmd.GetExtension(Command_DeckNewDir::ext), rc);
        case SessionCommand::DECK_DEL_DIR:
            return cmdDeckDelDir(cmd.GetExtension(Command_DeckDelDir::ext), rc);
        case SessionCommand::DECK_DEL:
            return cmdDeckDel(cmd.GetExtension(Command_DeckDel::ext), rc);
        case SessionCommand::DECK_UPLOAD:
            return cmdDeckUpload(cmd.GetExtension(Command_DeckUpload::ext), rc);
        case SessionCommand::DECK_DOWNLOAD:
            return cmdDeckDownload(cmd.GetExtension(Command_DeckDownload::ext), rc);
        case SessionCommand::REPLAY_LIST:
            return cmdReplayList(cmd.GetExtension(Command_ReplayList::ext), rc);
        case SessionCommand::REPLAY_DOWNLOAD:
            return cmdReplayDownload(cmd.GetExtension(Command_ReplayDownload::ext), rc);
        case SessionCommand::REPLAY_MODIFY_MATCH:
            return cmdReplayModifyMatch(cmd.GetExtension(Command_ReplayModifyMatch::ext), rc);
        case SessionCommand::REPLAY_DELETE_MATCH:
            return cmdReplayDeleteMatch(cmd.GetExtension(Command_ReplayDeleteMatch::ext), rc);
        case SessionCommand::REGISTER:
            return cmdRegisterAccount(cmd.GetExtension(Command_Register::ext), rc);
            break;
        case SessionCommand::ACTIVATE:
            return cmdActivateAccount(cmd.GetExtension(Command_Activate::ext), rc);
            break;
        case SessionCommand::FORGOT_PASSWORD_REQUEST:
            return cmdForgotPasswordRequest(cmd.GetExtension(Command_ForgotPasswordRequest::ext), rc);
            break;
        case SessionCommand::FORGOT_PASSWORD_RESET:
            return cmdForgotPasswordReset(cmd.GetExtension(Command_ForgotPasswordReset::ext), rc);
            break;
        case SessionCommand::FORGOT_PASSWORD_CHALLENGE:
            return cmdForgotPasswordChallenge(cmd.GetExtension(Command_ForgotPasswordChallenge::ext), rc);
            break;
        case SessionCommand::ACCOUNT_EDIT:
            return cmdAccountEdit(cmd.GetExtension(Command_AccountEdit::ext), rc);
        case SessionCommand::ACCOUNT_IMAGE:
            return cmdAccountImage(cmd.GetExtension(Command_AccountImage::ext), rc);
        case SessionCommand::ACCOUNT_PASSWORD:
            return cmdAccountPassword(cmd.GetExtension(Command_AccountPassword::ext), rc);
        case SessionCommand::REQUEST_PASSWORD_SALT:
            return cmdRequestPasswordSalt(cmd.GetExtension(Command_RequestPasswordSalt::ext), rc);
            break;
        default:
            return Response::RespFunctionNotAllowed;
    }
}

Response::ResponseCode AbstractServerSocketInterface::processExtendedModeratorCommand(int cmdType,
                                                                                      const ModeratorCommand &cmd,
                                                                                      ResponseContainer &rc)
{
    switch ((ModeratorCommand::ModeratorCommandType)cmdType) {
        case ModeratorCommand::BAN_FROM_SERVER:
            return cmdBanFromServer(cmd.GetExtension(Command_BanFromServer::ext), rc);
        case ModeratorCommand::BAN_HISTORY:
            return cmdGetBanHistory(cmd.GetExtension(Command_GetBanHistory::ext), rc);
        case ModeratorCommand::WARN_USER:
            return cmdWarnUser(cmd.GetExtension(Command_WarnUser::ext), rc);
        case ModeratorCommand::WARN_HISTORY:
            return cmdGetWarnHistory(cmd.GetExtension(Command_GetWarnHistory::ext), rc);
        case ModeratorCommand::WARN_LIST:
            return cmdGetWarnList(cmd.GetExtension(Command_GetWarnList::ext), rc);
        case ModeratorCommand::VIEWLOG_HISTORY:
            return cmdGetLogHistory(cmd.GetExtension(Command_ViewLogHistory::ext), rc);
        default:
            return Response::RespFunctionNotAllowed;
    }
}

Response::ResponseCode
AbstractServerSocketInterface::processExtendedAdminCommand(int cmdType, const AdminCommand &cmd, ResponseContainer &rc)
{
    switch ((AdminCommand::AdminCommandType)cmdType) {
        case AdminCommand::SHUTDOWN_SERVER:
            return cmdShutdownServer(cmd.GetExtension(Command_ShutdownServer::ext), rc);
        case AdminCommand::UPDATE_SERVER_MESSAGE:
            return cmdUpdateServerMessage(cmd.GetExtension(Command_UpdateServerMessage::ext), rc);
        case AdminCommand::RELOAD_CONFIG:
            return cmdReloadConfig(cmd.GetExtension(Command_ReloadConfig::ext), rc);
        case AdminCommand::ADJUST_MOD:
            return cmdAdjustMod(cmd.GetExtension(Command_AdjustMod::ext), rc);
        default:
            return Response::RespFunctionNotAllowed;
    }
}

Response::ResponseCode AbstractServerSocketInterface::cmdAddToList(const Command_AddToList &cmd, ResponseContainer &rc)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    QString list = nameFromStdString(cmd.list());
    QString user = nameFromStdString(cmd.user_name());

    if ((list != "buddy") && (list != "ignore"))
        return Response::RespContextError;

    if (list == "buddy")
        if (databaseInterface->isInBuddyList(QString::fromStdString(userInfo->name()), user))
            return Response::RespContextError;
    if (list == "ignore")
        if (databaseInterface->isInIgnoreList(QString::fromStdString(userInfo->name()), user))
            return Response::RespContextError;

    int id1 = userInfo->id();
    int id2 = sqlInterface->getUserIdInDB(user);
    if (id2 < 0)
        return Response::RespNameNotFound;
    if (id1 == id2)
        return Response::RespContextError;

    QSqlQuery *query =
        sqlInterface->prepareQuery("insert into {prefix}_" + list + "list (id_user1, id_user2) values(:id1, :id2)");
    query->bindValue(":id1", id1);
    query->bindValue(":id2", id2);
    if (!sqlInterface->execSqlQuery(query))
        return Response::RespInternalError;

    Event_AddToList event;
    event.set_list_name(cmd.list());
    event.mutable_user_info()->CopyFrom(databaseInterface->getUserData(user));
    rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event));

    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdRemoveFromList(const Command_RemoveFromList &cmd,
                                                                        ResponseContainer &rc)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    QString list = nameFromStdString(cmd.list());
    QString user = nameFromStdString(cmd.user_name());

    if ((list != "buddy") && (list != "ignore"))
        return Response::RespContextError;

    if (list == "buddy")
        if (!databaseInterface->isInBuddyList(QString::fromStdString(userInfo->name()), user))
            return Response::RespContextError;
    if (list == "ignore")
        if (!databaseInterface->isInIgnoreList(QString::fromStdString(userInfo->name()), user))
            return Response::RespContextError;

    int id1 = userInfo->id();
    int id2 = sqlInterface->getUserIdInDB(user);
    if (id2 < 0)
        return Response::RespNameNotFound;

    QSqlQuery *query =
        sqlInterface->prepareQuery("delete from {prefix}_" + list + "list where id_user1 = :id1 and id_user2 = :id2");
    query->bindValue(":id1", id1);
    query->bindValue(":id2", id2);
    if (!sqlInterface->execSqlQuery(query))
        return Response::RespInternalError;

    Event_RemoveFromList event;
    event.set_list_name(cmd.list());
    event.set_user_name(cmd.user_name());
    rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event));

    return Response::RespOk;
}

int AbstractServerSocketInterface::getDeckPathId(int basePathId, QStringList path)
{
    if (path.isEmpty())
        return 0;
    if (path[0].isEmpty())
        return 0;

    QSqlQuery *query = sqlInterface->prepareQuery("select id from {prefix}_decklist_folders where id_parent = "
                                                  ":id_parent and name = :name and id_user = :id_user");
    query->bindValue(":id_parent", basePathId);
    query->bindValue(":name", path.takeFirst());
    query->bindValue(":id_user", userInfo->id());
    if (!sqlInterface->execSqlQuery(query))
        return -1;
    if (!query->next())
        return -1;
    int id = query->value(0).toInt();
    if (path.isEmpty())
        return id;
    else
        return getDeckPathId(id, path);
}

int AbstractServerSocketInterface::getDeckPathId(const QString &path)
{
    return getDeckPathId(0, path.split("/"));
}

bool AbstractServerSocketInterface::deckListHelper(int folderId, ServerInfo_DeckStorage_Folder *folder)
{
    QSqlQuery *query = sqlInterface->prepareQuery(
        "select id, name from {prefix}_decklist_folders where id_parent = :id_parent and id_user = :id_user");
    query->bindValue(":id_parent", folderId);
    query->bindValue(":id_user", userInfo->id());
    if (!sqlInterface->execSqlQuery(query))
        return false;

    QMap<int, QString> results;
    while (query->next())
        results[query->value(0).toInt()] = query->value(1).toString();

    foreach (int key, results.keys()) {
        ServerInfo_DeckStorage_TreeItem *newItem = folder->add_items();
        newItem->set_id(key);
        newItem->set_name(results.value(key).toStdString());

        if (!deckListHelper(newItem->id(), newItem->mutable_folder()))
            return false;
    }

    query = sqlInterface->prepareQuery("select id, name, upload_time from {prefix}_decklist_files where id_folder = "
                                       ":id_folder and id_user = :id_user");
    query->bindValue(":id_folder", folderId);
    query->bindValue(":id_user", userInfo->id());
    if (!sqlInterface->execSqlQuery(query))
        return false;

    while (query->next()) {
        ServerInfo_DeckStorage_TreeItem *newItem = folder->add_items();
        newItem->set_id(query->value(0).toInt());
        newItem->set_name(query->value(1).toString().toStdString());

        ServerInfo_DeckStorage_File *newFile = newItem->mutable_file();
        newFile->set_creation_time(query->value(2).toDateTime().toSecsSinceEpoch());
    }

    return true;
}

// CHECK AUTHENTICATION!
// Also check for every function that data belonging to other users cannot be accessed.

Response::ResponseCode AbstractServerSocketInterface::cmdDeckList(const Command_DeckList & /*cmd*/,
                                                                  ResponseContainer &rc)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    sqlInterface->checkSql();

    Response_DeckList *re = new Response_DeckList;
    ServerInfo_DeckStorage_Folder *root = re->mutable_root();

    if (!deckListHelper(0, root))
        return Response::RespContextError;

    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdDeckNewDir(const Command_DeckNewDir &cmd,
                                                                    ResponseContainer & /*rc*/)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    sqlInterface->checkSql();

    QString path = nameFromStdString(cmd.path());
    int folderId = getDeckPathId(path);
    if (folderId == -1)
        return Response::RespNameNotFound;

    QString name = nameFromStdString(cmd.dir_name());
    if (path.length() + name.length() + 1 > MAX_NAME_LENGTH)
        return Response::RespContextError; // do not allow creation of paths that would be too long to delete

    QSqlQuery *query = sqlInterface->prepareQuery(
        "insert into {prefix}_decklist_folders (id_parent, id_user, name) values(:id_parent, :id_user, :name)");
    query->bindValue(":id_parent", folderId);
    query->bindValue(":id_user", userInfo->id());
    query->bindValue(":name", name);
    if (!sqlInterface->execSqlQuery(query))
        return Response::RespContextError;
    return Response::RespOk;
}

void AbstractServerSocketInterface::deckDelDirHelper(int basePathId)
{
    sqlInterface->checkSql();
    QSqlQuery *query =
        sqlInterface->prepareQuery("select id from {prefix}_decklist_folders where id_parent = :id_parent");
    query->bindValue(":id_parent", basePathId);
    sqlInterface->execSqlQuery(query);
    while (query->next())
        deckDelDirHelper(query->value(0).toInt());

    query = sqlInterface->prepareQuery("delete from {prefix}_decklist_files where id_folder = :id_folder");
    query->bindValue(":id_folder", basePathId);
    sqlInterface->execSqlQuery(query);

    query = sqlInterface->prepareQuery("delete from {prefix}_decklist_folders where id = :id");
    query->bindValue(":id", basePathId);
    sqlInterface->execSqlQuery(query);
}

void AbstractServerSocketInterface::sendServerMessage(const QString userName, const QString message)
{
    AbstractServerSocketInterface *user =
        static_cast<AbstractServerSocketInterface *>(server->getUsers().value(userName));
    if (!user)
        return;

    Event_UserMessage event;
    event.set_sender_name("Servatrice");
    event.set_receiver_name(userName.toStdString());
    event.set_message(message.toStdString());
    SessionEvent *se = user->prepareSessionEvent(event);
    user->sendProtocolItem(*se);
    delete se;
}

Response::ResponseCode AbstractServerSocketInterface::cmdDeckDelDir(const Command_DeckDelDir &cmd,
                                                                    ResponseContainer & /*rc*/)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    sqlInterface->checkSql();

    int basePathId = getDeckPathId(nameFromStdString(cmd.path()));
    if ((basePathId == -1) || (basePathId == 0))
        return Response::RespNameNotFound;
    deckDelDirHelper(basePathId);
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdDeckDel(const Command_DeckDel &cmd, ResponseContainer & /*rc*/)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    sqlInterface->checkSql();
    QSqlQuery *query =
        sqlInterface->prepareQuery("select id from {prefix}_decklist_files where id = :id and id_user = :id_user");
    query->bindValue(":id", cmd.deck_id());
    query->bindValue(":id_user", userInfo->id());
    sqlInterface->execSqlQuery(query);
    if (!query->next())
        return Response::RespNameNotFound;

    query = sqlInterface->prepareQuery("delete from {prefix}_decklist_files where id = :id");
    query->bindValue(":id", cmd.deck_id());
    sqlInterface->execSqlQuery(query);

    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdDeckUpload(const Command_DeckUpload &cmd,
                                                                    ResponseContainer &rc)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    if (!cmd.has_deck_list())
        return Response::RespInvalidData;

    sqlInterface->checkSql();

    QString deckStr = fileFromStdString(cmd.deck_list());
    DeckList deck;
    if (!deck.loadFromString_Native(deckStr))
        return Response::RespContextError;

    QString deckName = deck.getName();
    if (deckName.isEmpty())
        deckName = "Unnamed deck";

    if (cmd.has_path()) {
        int folderId = getDeckPathId(nameFromStdString(cmd.path()));
        if (folderId == -1)
            return Response::RespNameNotFound;

        QSqlQuery *query =
            sqlInterface->prepareQuery("insert into {prefix}_decklist_files (id_folder, id_user, name, upload_time, "
                                       "content) values(:id_folder, :id_user, :name, NOW(), :content)");
        query->bindValue(":id_folder", folderId);
        query->bindValue(":id_user", userInfo->id());
        query->bindValue(":name", deckName);
        query->bindValue(":content", deckStr);
        sqlInterface->execSqlQuery(query);

        Response_DeckUpload *re = new Response_DeckUpload;
        ServerInfo_DeckStorage_TreeItem *fileInfo = re->mutable_new_file();
        fileInfo->set_id(query->lastInsertId().toInt());
        fileInfo->set_name(deckName.toStdString());
        fileInfo->mutable_file()->set_creation_time(QDateTime::currentDateTime().toSecsSinceEpoch());
        rc.setResponseExtension(re);
    } else if (cmd.has_deck_id()) {
        QSqlQuery *query =
            sqlInterface->prepareQuery("update {prefix}_decklist_files set name=:name, upload_time=NOW(), "
                                       "content=:content where id = :id_deck and id_user = :id_user");
        query->bindValue(":id_deck", cmd.deck_id());
        query->bindValue(":id_user", userInfo->id());
        query->bindValue(":name", deckName);
        query->bindValue(":content", deckStr);
        sqlInterface->execSqlQuery(query);

        if (query->numRowsAffected() == 0)
            return Response::RespNameNotFound;

        Response_DeckUpload *re = new Response_DeckUpload;
        ServerInfo_DeckStorage_TreeItem *fileInfo = re->mutable_new_file();
        fileInfo->set_id(cmd.deck_id());
        fileInfo->set_name(deckName.toStdString());
        fileInfo->mutable_file()->set_creation_time(QDateTime::currentDateTime().toSecsSinceEpoch());
        rc.setResponseExtension(re);
    } else
        return Response::RespInvalidData;

    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdDeckDownload(const Command_DeckDownload &cmd,
                                                                      ResponseContainer &rc)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    DeckList *deck;
    try {
        deck = sqlInterface->getDeckFromDatabase(cmd.deck_id(), userInfo->id());
    } catch (Response::ResponseCode &r) {
        return r;
    }

    Response_DeckDownload *re = new Response_DeckDownload;
    re->set_deck(deck->writeToString_Native().toStdString());
    rc.setResponseExtension(re);
    delete deck;

    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdReplayList(const Command_ReplayList & /*cmd*/,
                                                                    ResponseContainer &rc)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    Response_ReplayList *re = new Response_ReplayList;

    QSqlQuery *query1 = sqlInterface->prepareQuery(
        "select a.id_game, a.replay_name, b.room_name, b.time_started, b.time_finished, b.descr, a.do_not_hide from "
        "{prefix}_replays_access a left join {prefix}_games b on b.id = a.id_game where a.id_player = :id_player and "
        "(a.do_not_hide = 1 or date_add(b.time_started, interval 7 day) > now())");
    query1->bindValue(":id_player", userInfo->id());
    sqlInterface->execSqlQuery(query1);
    while (query1->next()) {
        ServerInfo_ReplayMatch *matchInfo = re->add_match_list();

        const int gameId = query1->value(0).toInt();
        matchInfo->set_game_id(gameId);
        matchInfo->set_room_name(query1->value(2).toString().toStdString());
        const int timeStarted = query1->value(3).toDateTime().toSecsSinceEpoch();
        const int timeFinished = query1->value(4).toDateTime().toSecsSinceEpoch();
        matchInfo->set_time_started(timeStarted);
        matchInfo->set_length(timeFinished - timeStarted);
        matchInfo->set_game_name(query1->value(5).toString().toStdString());
        const QString replayName = query1->value(1).toString();
        matchInfo->set_do_not_hide(query1->value(6).toBool());

        {
            QSqlQuery *query2 =
                sqlInterface->prepareQuery("select player_name from {prefix}_games_players where id_game = :id_game");
            query2->bindValue(":id_game", gameId);
            sqlInterface->execSqlQuery(query2);
            while (query2->next())
                matchInfo->add_player_names(query2->value(0).toString().toStdString());
        }
        {
            QSqlQuery *query3 =
                sqlInterface->prepareQuery("select id, duration from {prefix}_replays where id_game = :id_game");
            query3->bindValue(":id_game", gameId);
            sqlInterface->execSqlQuery(query3);
            while (query3->next()) {
                ServerInfo_Replay *replayInfo = matchInfo->add_replay_list();
                replayInfo->set_replay_id(query3->value(0).toInt());
                replayInfo->set_replay_name(replayName.toStdString());
                replayInfo->set_duration(query3->value(1).toInt());
            }
        }
    }

    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdReplayDownload(const Command_ReplayDownload &cmd,
                                                                        ResponseContainer &rc)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    {
        QSqlQuery *query =
            sqlInterface->prepareQuery("select 1 from {prefix}_replays_access a left join {prefix}_replays b on "
                                       "a.id_game = b.id_game where b.id = :id_replay and a.id_player = :id_player");
        query->bindValue(":id_replay", cmd.replay_id());
        query->bindValue(":id_player", userInfo->id());
        if (!sqlInterface->execSqlQuery(query))
            return Response::RespInternalError;
        if (!query->next())
            return Response::RespAccessDenied;
    }

    QSqlQuery *query = sqlInterface->prepareQuery("select replay from {prefix}_replays where id = :id_replay");
    query->bindValue(":id_replay", cmd.replay_id());
    if (!sqlInterface->execSqlQuery(query))
        return Response::RespInternalError;
    if (!query->next())
        return Response::RespNameNotFound;

    QByteArray data = query->value(0).toByteArray();

    Response_ReplayDownload *re = new Response_ReplayDownload;
    re->set_replay_data(data.data(), data.size());
    rc.setResponseExtension(re);

    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdReplayModifyMatch(const Command_ReplayModifyMatch &cmd,
                                                                           ResponseContainer & /*rc*/)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    if (!sqlInterface->checkSql())
        return Response::RespInternalError;

    QSqlQuery *query = sqlInterface->prepareQuery("update {prefix}_replays_access set do_not_hide=:do_not_hide where "
                                                  "id_player = :id_player and id_game = :id_game");
    query->bindValue(":id_player", userInfo->id());
    query->bindValue(":id_game", cmd.game_id());
    query->bindValue(":do_not_hide", cmd.do_not_hide());

    if (!sqlInterface->execSqlQuery(query))
        return Response::RespInternalError;
    return query->numRowsAffected() > 0 ? Response::RespOk : Response::RespNameNotFound;
}

Response::ResponseCode AbstractServerSocketInterface::cmdReplayDeleteMatch(const Command_ReplayDeleteMatch &cmd,
                                                                           ResponseContainer & /*rc*/)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    if (!sqlInterface->checkSql())
        return Response::RespInternalError;

    QSqlQuery *query = sqlInterface->prepareQuery(
        "delete from {prefix}_replays_access where id_player = :id_player and id_game = :id_game");
    query->bindValue(":id_player", userInfo->id());
    query->bindValue(":id_game", cmd.game_id());

    if (!sqlInterface->execSqlQuery(query))
        return Response::RespInternalError;
    return query->numRowsAffected() > 0 ? Response::RespOk : Response::RespNameNotFound;
}

// MODERATOR FUNCTIONS.
// May be called by admins and moderators. Permission is checked by the calling function.
Response::ResponseCode AbstractServerSocketInterface::cmdGetLogHistory(const Command_ViewLogHistory &cmd,
                                                                       ResponseContainer &rc)
{

    QList<ServerInfo_ChatMessage> messageList;
    QString userName = nameFromStdString(cmd.user_name());
    QString ipAddress = nameFromStdString(cmd.ip_address());
    QString gameName = nameFromStdString(cmd.game_name());
    QString gameID = nameFromStdString(cmd.game_id());
    QString message = textFromStdString(cmd.message());
    bool chatType = false;
    bool gameType = false;
    bool roomType = false;

    for (int i = 0; i != cmd.log_location_size(); ++i) {
        if (nameFromStdString(cmd.log_location(i)).simplified() == "room")
            roomType = true;
        if (nameFromStdString(cmd.log_location(i)).simplified() == "game")
            gameType = true;
        if (nameFromStdString(cmd.log_location(i)).simplified() == "chat")
            chatType = true;
    }

    int dateRange = cmd.date_range();
    int maximumResults = cmd.maximum_results();

    Response_ViewLogHistory *re = new Response_ViewLogHistory;

    if (servatrice->getEnableLogQuery()) {
        QListIterator<ServerInfo_ChatMessage> messageIterator(sqlInterface->getMessageLogHistory(
            userName, ipAddress, gameName, gameID, message, chatType, gameType, roomType, dateRange, maximumResults));
        while (messageIterator.hasNext())
            re->add_log_message()->CopyFrom(messageIterator.next());
    } else {
        ServerInfo_ChatMessage chatMessage;

        // create dummy chat message for room tab in the event the query is for room messages (and possibly not others)
        chatMessage.set_time(QString(tr("Log query disabled, please contact server owner for details.")).toStdString());
        chatMessage.set_sender_id(QString("").toStdString());
        chatMessage.set_sender_name(QString("").toStdString());
        chatMessage.set_sender_ip(QString("").toStdString());
        chatMessage.set_message(QString("").toStdString());
        chatMessage.set_target_type(QString("room").toStdString());
        chatMessage.set_target_id(QString("").toStdString());
        chatMessage.set_target_name(QString("").toStdString());
        messageList << chatMessage;

        // create dummy chat message for room tab in the event the query is for game messages (and possibly not others)
        chatMessage.set_time(QString(tr("Log query disabled, please contact server owner for details.")).toStdString());
        chatMessage.set_sender_id(QString("").toStdString());
        chatMessage.set_sender_name(QString("").toStdString());
        chatMessage.set_sender_ip(QString("").toStdString());
        chatMessage.set_message(QString("").toStdString());
        chatMessage.set_target_type(QString("game").toStdString());
        chatMessage.set_target_id(QString("").toStdString());
        chatMessage.set_target_name(QString("").toStdString());
        messageList << chatMessage;

        // create dummy chat message for room tab in the event the query is for chat messages (and possibly not others)
        chatMessage.set_time(QString(tr("Log query disabled, please contact server owner for details.")).toStdString());
        chatMessage.set_sender_id(QString("").toStdString());
        chatMessage.set_sender_name(QString("").toStdString());
        chatMessage.set_sender_ip(QString("").toStdString());
        chatMessage.set_message(QString("").toStdString());
        chatMessage.set_target_type(QString("chat").toStdString());
        chatMessage.set_target_id(QString("").toStdString());
        chatMessage.set_target_name(QString("").toStdString());
        messageList << chatMessage;

        QListIterator<ServerInfo_ChatMessage> messageIterator(messageList);
        while (messageIterator.hasNext())
            re->add_log_message()->CopyFrom(messageIterator.next());
    }

    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdGetBanHistory(const Command_GetBanHistory &cmd,
                                                                       ResponseContainer &rc)
{
    QList<ServerInfo_Ban> banList;
    QString userName = nameFromStdString(cmd.user_name());

    Response_BanHistory *re = new Response_BanHistory;
    QListIterator<ServerInfo_Ban> banIterator(sqlInterface->getUserBanHistory(userName));
    while (banIterator.hasNext())
        re->add_ban_list()->CopyFrom(banIterator.next());
    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdGetWarnList(const Command_GetWarnList &cmd,
                                                                     ResponseContainer &rc)
{
    Response_WarnList *re = new Response_WarnList;

    QString officialWarnings = settingsCache->value("server/officialwarnings").toString();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList warningsList = officialWarnings.split(",", Qt::SkipEmptyParts);
#else
    QStringList warningsList = officialWarnings.split(",", QString::SkipEmptyParts);
#endif
    foreach (QString warning, warningsList) {
        re->add_warning(warning.toStdString());
    }
    re->set_user_name(nameFromStdString(cmd.user_name()).toStdString());
    re->set_user_clientid(nameFromStdString(cmd.user_clientid()).toStdString());
    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdGetWarnHistory(const Command_GetWarnHistory &cmd,
                                                                        ResponseContainer &rc)
{
    QList<ServerInfo_Warning> warnList;
    QString userName = nameFromStdString(cmd.user_name());

    Response_WarnHistory *re = new Response_WarnHistory;
    QListIterator<ServerInfo_Warning> warnIterator(sqlInterface->getUserWarnHistory(userName));
    while (warnIterator.hasNext())
        re->add_warn_list()->CopyFrom(warnIterator.next());
    rc.setResponseExtension(re);
    return Response::RespOk;
}

void AbstractServerSocketInterface::removeSaidMessages(const QString &userName, int amount)
{
    for (auto *room : rooms.values()) {
        room->removeSaidMessages(userName, amount);
    }
}

Response::ResponseCode AbstractServerSocketInterface::cmdWarnUser(const Command_WarnUser &cmd,
                                                                  ResponseContainer & /*rc*/)
{
    if (!sqlInterface->checkSql()) { // sql database is required, without database there are no moderators anyway
        return Response::RespInternalError;
    }

    QString userName = nameFromStdString(cmd.user_name()).simplified();
    QString warningReason = textFromStdString(cmd.reason()).simplified();
    QString clientId = nameFromStdString(cmd.clientid()).simplified();
    QString sendingModerator = QString::fromStdString(userInfo->name()).simplified();
    int amountRemove = cmd.remove_messages();
    if (amountRemove != 0) {
        removeSaidMessages(userName, amountRemove);
    }

    if (sqlInterface->addWarning(userName, sendingModerator, warningReason, clientId)) {
        servatrice->clientsLock.lockForRead();
        AbstractServerSocketInterface *user =
            static_cast<AbstractServerSocketInterface *>(server->getUsers().value(userName));
        QList<QString> moderatorList = server->getOnlineModeratorList();
        servatrice->clientsLock.unlock();

        if (user != nullptr) {
            Event_NotifyUser event;
            event.set_type(Event_NotifyUser::WARNING);
            event.set_warning_reason(warningReason.toStdString());
            SessionEvent *se = user->prepareSessionEvent(event);
            user->sendProtocolItem(*se);
            delete se;
        }

        for (QString &moderator : moderatorList) {
            QString notificationMessage = sendingModerator + " has sent a warning with the following information";
            notificationMessage.append("\n    Username: " + userName);
            notificationMessage.append("\n    Reason: " + warningReason);
            sendServerMessage(moderator.simplified(), notificationMessage);
        }

        return Response::RespOk;
    } else {
        return Response::RespInternalError;
    }
}

Response::ResponseCode AbstractServerSocketInterface::cmdBanFromServer(const Command_BanFromServer &cmd,
                                                                       ResponseContainer & /*rc*/)
{
    if (!sqlInterface->checkSql())
        return Response::RespInternalError;

    QString userName = nameFromStdString(cmd.user_name()).simplified();
    QString address = nameFromStdString(cmd.address()).simplified();
    QString clientId = nameFromStdString(cmd.clientid()).simplified();
    QString visibleReason = textFromStdString(cmd.visible_reason());

    if (userName.isEmpty() && address.isEmpty() && clientId.isEmpty())
        return Response::RespOk;

    int amountRemove = cmd.remove_messages();
    if (amountRemove != 0) {
        removeSaidMessages(userName, amountRemove);
    }
    QString trustedSources = settingsCache->value("server/trusted_sources", "127.0.0.1,::1").toString();
    int minutes = cmd.minutes();
    if (trustedSources.contains(address, Qt::CaseInsensitive))
        address = "";

    QSqlQuery *query = sqlInterface->prepareQuery(
        "insert into {prefix}_bans (user_name, ip_address, id_admin, time_from, minutes, reason, visible_reason, "
        "clientid) values(:user_name, :ip_address, :id_admin, NOW(), :minutes, :reason, :visible_reason, :client_id)");
    query->bindValue(":user_name", userName);
    query->bindValue(":ip_address", address);
    query->bindValue(":id_admin", userInfo->id());
    query->bindValue(":minutes", minutes);
    query->bindValue(":reason", textFromStdString(cmd.reason()));
    query->bindValue(":visible_reason", visibleReason);
    query->bindValue(":client_id", nameFromStdString(cmd.clientid()));
    sqlInterface->execSqlQuery(query);

    servatrice->clientsLock.lockForRead();
    QList<QString> moderatorList = server->getOnlineModeratorList();
    QList<AbstractServerSocketInterface *> userList = servatrice->getUsersWithAddressAsList(QHostAddress(address));

    if (!userName.isEmpty()) {
        AbstractServerSocketInterface *user =
            static_cast<AbstractServerSocketInterface *>(server->getUsers().value(userName));
        if (user && !userList.contains(user))
            userList.append(user);
    }

    if (userName.isEmpty() && address.isEmpty() && (!clientId.isEmpty())) {
        QSqlQuery *clientIdQuery =
            sqlInterface->prepareQuery("select name from {prefix}_users where clientid = :client_id");
        clientIdQuery->bindValue(":client_id", nameFromStdString(cmd.clientid()));
        sqlInterface->execSqlQuery(clientIdQuery);
        if (!sqlInterface->execSqlQuery(clientIdQuery)) {
            qDebug("ClientID username ban lookup failed: SQL Error");
        } else {
            while (clientIdQuery->next()) {
                userName = clientIdQuery->value(0).toString();
                AbstractServerSocketInterface *user =
                    static_cast<AbstractServerSocketInterface *>(server->getUsers().value(userName));
                if (user && !userList.contains(user))
                    userList.append(user);
            }
        }
    }
    servatrice->clientsLock.unlock();

    if (!userList.isEmpty()) {
        Event_ConnectionClosed event;
        event.set_reason(Event_ConnectionClosed::BANNED);
        if (cmd.has_visible_reason())
            event.set_reason_str(visibleReason.toStdString());
        if (minutes)
            event.set_end_time(QDateTime::currentDateTime().addSecs(60 * minutes).toSecsSinceEpoch());
        for (int i = 0; i < userList.size(); ++i) {
            SessionEvent *se = userList[i]->prepareSessionEvent(event);
            userList[i]->sendProtocolItem(*se);
            delete se;
            QMetaObject::invokeMethod(userList[i], "prepareDestroy", Qt::QueuedConnection);
        }
    }

    for (QString &moderator : moderatorList) {
        QString notificationMessage =
            QString::fromStdString(userInfo->name()).simplified() + " has placed a ban with the following information";
        if (!userName.isEmpty())
            notificationMessage.append("\n    Username: " + userName);
        if (!address.isEmpty())
            notificationMessage.append("\n    IP Address: " + address);
        if (!clientId.isEmpty())
            notificationMessage.append("\n    Client ID: " + clientId);

        notificationMessage.append("\n    Length: " + QString::number(minutes) + " minute(s)");
        notificationMessage.append("\n    Internal Reason: " + textFromStdString(cmd.reason()));
        notificationMessage.append("\n    Visible Reason: " + textFromStdString(cmd.visible_reason()));
        sendServerMessage(moderator.simplified(), notificationMessage);
    }

    return Response::RespOk;
}

QPair<QString, QString> AbstractServerSocketInterface::parseEmailAddress(const QString &emailAddress)
{
    // https://www.regular-expressions.info/email.html
    static const QRegularExpression emailRegex(R"(^([A-Z0-9._%+-]+)@([A-Z0-9.-]+\.[A-Z]{2,})$)",
                                               QRegularExpression::CaseInsensitiveOption);
    const auto match = emailRegex.match(emailAddress);

    if (emailAddress.isEmpty() || !match.hasMatch()) {
        return {};
    }

    QString capturedEmailUser = match.captured(1);
    QString capturedEmailAddressDomain = match.captured(2);

    // Replace googlemail.com with gmail.com, as is standard nowadays
    // https://www.gmass.co/blog/domains-gmail-com-googlemail-com-and-google-com/
    if (capturedEmailAddressDomain.toLower() == "googlemail.com") {
        capturedEmailAddressDomain = "gmail.com";
    }

    // Trim out dots and pluses from Google/Gmail domains
    if (capturedEmailAddressDomain.toLower() == "gmail.com") {
        // Remove all content after first plus sign (as unnecessary with gmail)
        // https://gmail.googleblog.com/2008/03/2-hidden-ways-to-get-more-from-your.html
        const int firstPlusSign = capturedEmailUser.indexOf("+");
        if (firstPlusSign != -1) {
            capturedEmailUser = capturedEmailUser.left(firstPlusSign);
        }

        // Remove all periods (as unnecessary with gmail)
        // https://gmail.googleblog.com/2008/03/2-hidden-ways-to-get-more-from-your.html
        capturedEmailUser.replace(".", "");
    }

    return {capturedEmailUser, capturedEmailAddressDomain};
}

Response::ResponseCode AbstractServerSocketInterface::cmdRegisterAccount(const Command_Register &cmd,
                                                                         ResponseContainer &rc)
{
    QString userName = nameFromStdString(cmd.user_name());
    QString clientId = nameFromStdString(cmd.clientid());
    qDebug() << "Got register command for user:" << userName;

    bool registrationEnabled = settingsCache->value("registration/enabled", false).toBool();
    if (!registrationEnabled) {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Server functionality disabled", false);

        return Response::RespRegistrationDisabled;
    }

    const QString emailBlackList = servatrice->getEmailBlackList();
    const QString emailWhiteList = servatrice->getEmailWhiteList();
    auto parsedEmailAddress = parseEmailAddress(nameFromStdString(cmd.email()));
    const QString emailUser = parsedEmailAddress.first;
    const QString emailDomain = parsedEmailAddress.second;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const QStringList emailBlackListFilters = emailBlackList.split(",", Qt::SkipEmptyParts);
    const QStringList emailWhiteListFilters = emailWhiteList.split(",", Qt::SkipEmptyParts);
#else
    const QStringList emailBlackListFilters = emailBlackList.split(",", QString::SkipEmptyParts);
    const QStringList emailWhiteListFilters = emailWhiteList.split(",", QString::SkipEmptyParts);
#endif

    bool requireEmailForRegistration = settingsCache->value("registration/requireemail", true).toBool();
    if (requireEmailForRegistration && emailUser.isEmpty()) {
        return Response::RespEmailRequiredToRegister;
    }

    // If a whitelist exists, ensure the email address domain IS in the whitelist
    if (!emailWhiteListFilters.isEmpty() && !emailWhiteListFilters.contains(emailDomain, Qt::CaseInsensitive)) {
        if (servatrice->getEnableRegistrationAudit()) {
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Email used is not whitelisted", false);
        }
        auto *re = new Response_Register;
        re->set_denied_reason_str(
            "The email address provider used during registration has not been approved for use on this server.");
        rc.setResponseExtension(re);
        return Response::RespEmailBlackListed;
    }

    // If a blacklist exists, ensure the email address domain is NOT in the blacklist
    if (!emailBlackListFilters.isEmpty() && emailBlackListFilters.contains(emailDomain, Qt::CaseInsensitive)) {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Email used is blacklisted", false);

        return Response::RespEmailBlackListed;
    }

    // TODO: Move this method outside of the db interface
    QString errorString;
    if (!sqlInterface->usernameIsValid(userName, errorString)) {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Username is invalid", false);

        Response_Register *re = new Response_Register;
        re->set_denied_reason_str(errorString.toStdString());
        rc.setResponseExtension(re);
        return Response::RespUsernameInvalid;
    }

    if (userName.toLower().simplified() == "servatrice") {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Username is invalid", false);

        return Response::RespUsernameInvalid;
    }

    if (sqlInterface->userExists(userName)) {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Username already exists", false);

        return Response::RespUserAlreadyExists;
    }

    QString emailAddress = emailUser + "@" + emailDomain;
    if (servatrice->getMaxAccountsPerEmail() > 0 &&
        sqlInterface->checkNumberOfUserAccounts(emailAddress) >= servatrice->getMaxAccountsPerEmail()) {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Too many usernames registered with this email address",
                                         false);

        return Response::RespTooManyRequests;
    }

    QString banReason;
    int banSecondsRemaining;
    if (sqlInterface->checkUserIsBanned(this->getAddress(), userName, clientId, banReason, banSecondsRemaining)) {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "User is banned", false);

        Response_Register *re = new Response_Register;
        re->set_denied_reason_str(banReason.toStdString());
        if (banSecondsRemaining != 0)
            re->set_denied_end_time(QDateTime::currentDateTime().addSecs(banSecondsRemaining).toSecsSinceEpoch());
        rc.setResponseExtension(re);
        return Response::RespUserIsBanned;
    }

    if (tooManyRegistrationAttempts(this->getAddress())) {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Too many registration attempts from this ip address",
                                         false);

        return Response::RespTooManyRequests;
    }

    QString realName = nameFromStdString(cmd.real_name());
    QString country = nameFromStdString(cmd.country());
    QString password;
    bool passwordNeedsHash = false;
    if (cmd.has_password()) {
        if (cmd.password().length() > MAX_NAME_LENGTH)
            return Response::RespRegistrationFailed;
        password = QString::fromStdString(cmd.password());
        passwordNeedsHash = true;
        if (!isPasswordLongEnough(password.length())) {
            if (servatrice->getEnableRegistrationAudit()) {
                sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                             "REGISTER_ACCOUNT", "Password is too short", false);
            }
            return Response::RespPasswordTooShort;
        }
    } else if (cmd.hashed_password().length() > MAX_NAME_LENGTH) {
        return Response::RespRegistrationFailed;
    } else {
        password = QString::fromStdString(cmd.hashed_password());
    }

    bool requireEmailActivation = settingsCache->value("registration/requireemailactivation", true).toBool();
    bool regSucceeded = sqlInterface->registerUser(userName, realName, password, passwordNeedsHash, emailAddress,
                                                   country, !requireEmailActivation);

    if (regSucceeded) {
        qDebug() << "Accepted register command for user:" << userName;
        if (requireEmailActivation) {
            QSqlQuery *query =
                sqlInterface->prepareQuery("insert into {prefix}_activation_emails (name) values(:name)");
            query->bindValue(":name", userName);
            if (!sqlInterface->execSqlQuery(query))
                return Response::RespRegistrationFailed;

            if (servatrice->getEnableRegistrationAudit())
                sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                             "REGISTER_ACCOUNT", "", true);

            return Response::RespRegistrationAcceptedNeedsActivation;
        } else {

            if (servatrice->getEnableRegistrationAudit())
                sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                             "REGISTER_ACCOUNT", "", true);

            return Response::RespRegistrationAccepted;
        }
    } else {
        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "REGISTER_ACCOUNT", "Unknown reason for failure", false);

        return Response::RespRegistrationFailed;
    }
}

bool AbstractServerSocketInterface::tooManyRegistrationAttempts(const QString &ipAddress)
{
    // TODO: implement
    Q_UNUSED(ipAddress);
    return false;
}

Response::ResponseCode AbstractServerSocketInterface::cmdActivateAccount(const Command_Activate &cmd,
                                                                         ResponseContainer & /*rc*/)
{
    QString userName = nameFromStdString(cmd.user_name());
    QString token = nameFromStdString(cmd.token());
    QString clientId = nameFromStdString(cmd.clientid());

    if (clientId.isEmpty())
        clientId = "UNKNOWN";

    if (sqlInterface->activateUser(userName, token)) {
        qDebug() << "Accepted activation for user" << userName;

        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "ACTIVATE_ACCOUNT", "", true);

        return Response::RespActivationAccepted;
    } else {
        qDebug() << "Failed activation for user" << userName;

        if (servatrice->getEnableRegistrationAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "ACTIVATE_ACCOUNT", "Failed to activate account, incorrect activation token",
                                         false);

        return Response::RespActivationFailed;
    }
}

Response::ResponseCode AbstractServerSocketInterface::cmdAccountEdit(const Command_AccountEdit &cmd,
                                                                     ResponseContainer & /* rc */)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    QString realName = nameFromStdString(cmd.real_name());
    QString emailAddress = nameFromStdString(cmd.email());
    QString country = nameFromStdString(cmd.country());

    bool checkedPassword = false;
    QString userName = QString::fromStdString(userInfo->name());
    if (cmd.has_password_check()) {
        if (cmd.password_check().length() > MAX_NAME_LENGTH)
            return Response::RespWrongPassword;
        QString password = QString::fromStdString(cmd.password_check());
        QString clientId = QString::fromStdString(userInfo->clientid());
        QString reasonStr{};
        int secondsLeft{};
        AuthenticationResult checkStatus =
            databaseInterface->checkUserPassword(this, userName, password, clientId, reasonStr, secondsLeft, true);
        if (checkStatus == PasswordRight) {
            checkedPassword = true;
        } else {
            // the user already logged in with this info, the only change is their password
            return Response::RespWrongPassword;
        }
    }

    QStringList queryList({});
    if (cmd.has_real_name()) {
        queryList << "realname=:realName";
    }
    if (cmd.has_email()) {
        // a real password is required in order to change the email address
        if (usingRealPassword || checkedPassword) {
            queryList << "email=:email";
        } else {
            return Response::RespFunctionNotAllowed;
        }
    }
    if (cmd.has_country()) {
        queryList << "country=:country";
    }

    if (queryList.isEmpty())
        return Response::RespOk;

    QString queryText = QString("update {prefix}_users set %1 where name=:userName").arg(queryList.join(", "));
    QSqlQuery *query = sqlInterface->prepareQuery(queryText);
    if (cmd.has_real_name()) {
        auto _realName = nameFromStdString(cmd.real_name());
        query->bindValue(":realName", _realName);
    }
    if (cmd.has_email()) {
        auto _emailAddress = nameFromStdString(cmd.email());
        query->bindValue(":email", _emailAddress);
    }
    if (cmd.has_country()) {
        auto _country = nameFromStdString(cmd.country());
        query->bindValue(":country", _country);
    }
    query->bindValue(":userName", userName);

    if (!sqlInterface->execSqlQuery(query))
        return Response::RespInternalError;

    if (cmd.has_real_name()) {
        userInfo->set_real_name(realName.toStdString());
    }
    if (cmd.has_email()) {
        userInfo->set_email(emailAddress.toStdString());
    }
    if (cmd.has_country()) {
        userInfo->set_country(country.toStdString());
    }

    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdAccountImage(const Command_AccountImage &cmd,
                                                                      ResponseContainer & /* rc */)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    size_t length = qMin(cmd.image().length(), (size_t)MAX_FILE_LENGTH);
    QByteArray image(cmd.image().c_str(), length);
    int id = userInfo->id();

    QSqlQuery *query = sqlInterface->prepareQuery("update {prefix}_users set avatar_bmp=:image where id=:id");
    query->bindValue(":image", image);
    query->bindValue(":id", id);
    if (!sqlInterface->execSqlQuery(query))
        return Response::RespInternalError;

    userInfo->set_avatar_bmp(cmd.image().c_str(), length);
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdAccountPassword(const Command_AccountPassword &cmd,
                                                                         ResponseContainer & /* rc */)
{
    if (authState != PasswordRight)
        return Response::RespFunctionNotAllowed;

    if (cmd.old_password().length() > MAX_NAME_LENGTH)
        return Response::RespWrongPassword;
    QString oldPassword = QString::fromStdString(cmd.old_password());
    QString newPassword;
    bool newPasswordNeedsHash = false;
    if (cmd.has_new_password()) {
        if (cmd.new_password().length() > MAX_NAME_LENGTH)
            return Response::RespContextError;
        newPassword = QString::fromStdString(cmd.new_password());
        newPasswordNeedsHash = true;
        if (!isPasswordLongEnough(newPassword.length()))
            return Response::RespPasswordTooShort;
    } else if (cmd.hashed_new_password().length() > MAX_NAME_LENGTH) {
        return Response::RespContextError;
    } else {
        newPassword = QString::fromStdString(cmd.hashed_new_password());
    }

    QString userName = QString::fromStdString(userInfo->name());
    if (!databaseInterface->changeUserPassword(userName, oldPassword, true, newPassword, newPasswordNeedsHash))
        return Response::RespWrongPassword;

    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdForgotPasswordRequest(const Command_ForgotPasswordRequest &cmd,
                                                                               ResponseContainer &rc)
{
    const QString userName = nameFromStdString(cmd.user_name());
    const QString clientId = nameFromStdString(cmd.clientid());

    qDebug() << "Received reset password request from user:" << userName;

    if (!servatrice->getEnableForgotPassword()) {
        if (servatrice->getEnableForgotPasswordAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET_REQUEST", "Server functionality disabled", false);

        return Response::RespFunctionNotAllowed;
    }

    if (servatrice->getEnableForgotPasswordChallenge()) {
        Response_ForgotPasswordRequest *re = new Response_ForgotPasswordRequest;
        re->set_challenge_email(true);
        rc.setResponseExtension(re);
        return Response::RespOk;
    }

    if (!sqlInterface->userExists(userName)) {
        if (servatrice->getEnableForgotPasswordAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET_REQUEST", "User does not exist", false);

        return Response::RespFunctionNotAllowed;
    }

    return continuePasswordRequest(userName, clientId, rc);
}

Response::ResponseCode AbstractServerSocketInterface::continuePasswordRequest(const QString &userName,
                                                                              const QString &clientId,
                                                                              ResponseContainer &rc,
                                                                              bool challenged)
{
    if (sqlInterface->doesForgotPasswordExist(userName)) {

        if (servatrice->getEnableForgotPasswordAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET_REQUEST", "Request already exists", true);

        Response_ForgotPasswordRequest *re = new Response_ForgotPasswordRequest;
        re->set_challenge_email(false);
        rc.setResponseExtension(re);
        return Response::RespOk;
    }

    QString banReason;
    int banTimeRemaining;
    if (sqlInterface->checkUserIsBanned(this->getAddress(), userName, clientId, banReason, banTimeRemaining)) {
        if (servatrice->getEnableForgotPasswordAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET_REQUEST", "User is banned", false);

        return Response::RespFunctionNotAllowed;
    }

    if (!sqlInterface->addForgotPassword(userName)) {
        if (servatrice->getEnableForgotPasswordAudit()) {
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET_REQUEST", "Failed to create password reset", false);
        }
        return Response::RespFunctionNotAllowed;
    }

    if (servatrice->getEnableForgotPasswordAudit()) {
        QString details =
            challenged ? "Request does not exist, challenge passed" : "Request does not exist, challenge not requested";
        sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                     "PASSWORD_RESET_REQUEST", details, true);
    }

    Response_ForgotPasswordRequest *re = new Response_ForgotPasswordRequest;
    re->set_challenge_email(false);
    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdForgotPasswordReset(const Command_ForgotPasswordReset &cmd,
                                                                             ResponseContainer &rc)
{
    Q_UNUSED(rc);
    QString userName = nameFromStdString(cmd.user_name());
    QString clientId = nameFromStdString(cmd.clientid());
    qDebug() << "Received reset password reset from user:" << userName;

    if (!sqlInterface->doesForgotPasswordExist(userName)) {
        if (servatrice->getEnableForgotPasswordAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET", "Request does not exist for user", false);

        return Response::RespFunctionNotAllowed;
    }

    if (!sqlInterface->validateTableColumnStringData("{prefix}_users", "token", userName,
                                                     nameFromStdString(cmd.token()))) {
        if (servatrice->getEnableForgotPasswordAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), userName.simplified(),
                                         "PASSWORD_RESET", "Failed token validation", false);

        return Response::RespFunctionNotAllowed;
    }

    QString password;
    bool passwordNeedsHash = false;
    if (cmd.has_new_password()) {
        if (cmd.new_password().length() > MAX_NAME_LENGTH)
            return Response::RespContextError;
        password = QString::fromStdString(cmd.new_password());
        passwordNeedsHash = true;
    } else if (cmd.hashed_new_password().length() > MAX_NAME_LENGTH) {
        return Response::RespContextError;
    } else {
        password = QString::fromStdString(cmd.hashed_new_password());
    }

    if (sqlInterface->changeUserPassword(nameFromStdString(cmd.user_name()), password, passwordNeedsHash)) {
        if (servatrice->getEnableForgotPasswordAudit())
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET", "", true);

        sqlInterface->removeForgotPassword(nameFromStdString(cmd.user_name()));
        return Response::RespOk;
    }

    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode
AbstractServerSocketInterface::cmdForgotPasswordChallenge(const Command_ForgotPasswordChallenge &cmd,
                                                          ResponseContainer &rc)
{
    const QString userName = nameFromStdString(cmd.user_name());
    const QString clientId = nameFromStdString(cmd.clientid());

    qDebug() << "Received reset password challenge from user:" << userName;

    if (!servatrice->getEnableForgotPasswordChallenge()) {
        if (servatrice->getEnableForgotPasswordAudit()) {
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET_CHALLENGE", "Feature not enabled", false);
        }
        return Response::RespFunctionNotAllowed;
    }

    if (!sqlInterface->userExists(userName)) {
        if (servatrice->getEnableForgotPasswordAudit()) {
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET_CHALLENGE", "User does not exist", false);
        }
        return Response::RespFunctionNotAllowed;
    }

    if (!sqlInterface->validateTableColumnStringData("{prefix}_users", "email", userName,
                                                     nameFromStdString(cmd.email()))) {
        if (servatrice->getEnableForgotPasswordAudit()) {
            sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                         "PASSWORD_RESET_CHALLENGE", "Failed to answer email challenge question",
                                         false);
        }
        return Response::RespFunctionNotAllowed;
    }

    if (servatrice->getEnableForgotPasswordAudit()) {
        sqlInterface->addAuditRecord(userName.simplified(), this->getAddress(), clientId.simplified(),
                                     "PASSWORD_RESET_CHALLENGE", "", true);
    }
    return continuePasswordRequest(userName, clientId, rc, true);
}

Response::ResponseCode AbstractServerSocketInterface::cmdRequestPasswordSalt(const Command_RequestPasswordSalt &cmd,
                                                                             ResponseContainer &rc)
{
    const QString userName = nameFromStdString(cmd.user_name());
    QString passwordSalt = sqlInterface->getUserSalt(userName);
    if (passwordSalt.isEmpty()) {
        if (server->getRegOnlyServerEnabled()) {
            return Response::RespRegistrationRequired;
        } else {
            // user does not exist but is allowed to log in unregistered without password
            return Response::RespOk;
        }
    }
    auto *re = new Response_PasswordSalt;
    re->set_password_salt(passwordSalt.toStdString());
    rc.setResponseExtension(re);
    return Response::RespOk;
}

// ADMIN FUNCTIONS.
// Permission is checked by the calling function.

Response::ResponseCode
AbstractServerSocketInterface::cmdUpdateServerMessage(const Command_UpdateServerMessage & /*cmd*/,
                                                      ResponseContainer & /*rc*/)
{
    QMetaObject::invokeMethod(server, "updateLoginMessage");
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdShutdownServer(const Command_ShutdownServer &cmd,
                                                                        ResponseContainer & /*rc*/)
{
    QMetaObject::invokeMethod(server, "scheduleShutdown", Q_ARG(QString, textFromStdString(cmd.reason())),
                              Q_ARG(int, cmd.minutes()));
    return Response::RespOk;
}

Response::ResponseCode AbstractServerSocketInterface::cmdReloadConfig(const Command_ReloadConfig & /* cmd */,
                                                                      ResponseContainer & /*rc*/)
{
    logDebugMessage("Received admin command: reloading configuration");
    settingsCache->sync();
    QMetaObject::invokeMethod(server, "setRequiredFeatures", Q_ARG(QString, server->getRequiredFeatures()));
    return Response::RespOk;
}

bool AbstractServerSocketInterface::addAdminFlagToUser(const QString &userName, int flag)
{
    QSqlQuery *query =
        sqlInterface->prepareQuery("update {prefix}_users set admin = (admin | :adminlevel) where name = :username");
    query->bindValue(":adminlevel", flag);
    query->bindValue(":username", userName);
    if (!sqlInterface->execSqlQuery(query)) {
        logger->logMessage(QString("Failed to promote user %1: %2").arg(userName).arg(query->lastError().text()));
        return false;
    }

    AbstractServerSocketInterface *user =
        static_cast<AbstractServerSocketInterface *>(server->getUsers().value(userName));
    if (user) {
        Event_NotifyUser event;
        event.set_type(Event_NotifyUser::PROMOTED);
        SessionEvent *se = user->prepareSessionEvent(event);
        user->sendProtocolItem(*se);
        delete se;
    }

    return true;
}

bool AbstractServerSocketInterface::removeAdminFlagFromUser(const QString &userName, int flag)
{
    QSqlQuery *query =
        sqlInterface->prepareQuery("update {prefix}_users set admin = (admin & ~ :adminlevel) where name = :username");
    query->bindValue(":adminlevel", flag);
    query->bindValue(":username", userName);
    if (!sqlInterface->execSqlQuery(query)) {
        logger->logMessage(QString("Failed to demote user %1: %2").arg(userName).arg(query->lastError().text()));
        return false;
    }

    AbstractServerSocketInterface *user =
        static_cast<AbstractServerSocketInterface *>(server->getUsers().value(userName));
    if (user) {
        Event_ConnectionClosed event;
        event.set_reason(Event_ConnectionClosed::DEMOTED);
        event.set_reason_str("Your moderator and/or judge status has been revoked.");
        event.set_end_time(QDateTime::currentDateTime().toSecsSinceEpoch());

        SessionEvent *se = user->prepareSessionEvent(event);
        user->sendProtocolItem(*se);
        delete se;
    }

    QMetaObject::invokeMethod(user, "prepareDestroy", Qt::QueuedConnection);
    return true;
}

Response::ResponseCode AbstractServerSocketInterface::cmdAdjustMod(const Command_AdjustMod &cmd,
                                                                   ResponseContainer & /*rc*/)
{

    QString userName = nameFromStdString(cmd.user_name());

    if (cmd.has_should_be_mod()) {
        if (cmd.should_be_mod()) {
            if (!addAdminFlagToUser(userName, 2)) {
                return Response::RespInternalError;
            }
        } else {
            if (!removeAdminFlagFromUser(userName, 2)) {
                return Response::RespInternalError;
            }
        }
    }

    if (cmd.has_should_be_judge()) {
        if (cmd.should_be_judge()) {
            if (!addAdminFlagToUser(userName, 4)) {
                return Response::RespInternalError;
            }
        } else {
            if (!removeAdminFlagFromUser(userName, 4)) {
                return Response::RespInternalError;
            }
        }
    }

    return Response::RespOk;
}

TcpServerSocketInterface::TcpServerSocketInterface(Servatrice *_server,
                                                   Servatrice_DatabaseInterface *_databaseInterface,
                                                   QObject *parent)
    : AbstractServerSocketInterface(_server, _databaseInterface, parent), messageInProgress(false),
      handshakeStarted(false)
{
    socket = new QTcpSocket(this);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(catchSocketDisconnected()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this,
            SLOT(catchSocketError(QAbstractSocket::SocketError)));
#else
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(catchSocketError(QAbstractSocket::SocketError)));
#endif
}

TcpServerSocketInterface::~TcpServerSocketInterface()
{
    logger->logMessage("TcpServerSocketInterface destructor", this);

    flushOutputQueue();
}

void TcpServerSocketInterface::initConnection(int socketDescriptor)
{
    // Add this object to the server's list of connections before it can receive socket events.
    // Otherwise, in case a of a socket error, it could be removed from the list before it is added.
    server->addClient(this);

    socket->setSocketDescriptor(socketDescriptor);
    logger->logMessage(QString("Incoming connection: %1").arg(socket->peerAddress().toString()), this);
    initSessionDeprecated();
}

void TcpServerSocketInterface::initSessionDeprecated()
{
    // dirty hack to make v13 client display the correct error message

    QByteArray buf;
    buf.append("<?xml version=\"1.0\"?><cockatrice_server_stream version=\"14\">");
    writeToSocket(buf);
    flushSocket();
}

void TcpServerSocketInterface::flushOutputQueue()
{
    QMutexLocker locker(&outputQueueMutex);
    if (outputQueue.isEmpty())
        return;

    int totalBytes = 0;
    while (!outputQueue.isEmpty()) {
        ServerMessage item = outputQueue.takeFirst();
        locker.unlock();

        QByteArray buf;
#if GOOGLE_PROTOBUF_VERSION > 3001000
        unsigned int size = static_cast<unsigned int>(item.ByteSizeLong());
#else
        unsigned int size = static_cast<unsigned int>(item.ByteSize());
#endif
        buf.resize(size + 4);
        item.SerializeToArray(buf.data() + 4, size);
        buf.data()[3] = (unsigned char)size;
        buf.data()[2] = (unsigned char)(size >> 8);
        buf.data()[1] = (unsigned char)(size >> 16);
        buf.data()[0] = (unsigned char)(size >> 24);
        // In case socket->write() calls catchSocketError(), the mutex must not be locked during this call.
        writeToSocket(buf);

        totalBytes += size + 4;
        locker.relock();
    }
    locker.unlock();
    emit incTxBytes(totalBytes);
    // see above wrt mutex
    flushSocket();
}

void TcpServerSocketInterface::readClient()
{
    QByteArray data = socket->readAll();
    servatrice->incRxBytes(data.size());
    inputBuffer.append(data);

    do {
        if (!messageInProgress) {
            if (inputBuffer.size() >= 4) {
                messageLength = (((quint32)(unsigned char)inputBuffer[0]) << 24) +
                                (((quint32)(unsigned char)inputBuffer[1]) << 16) +
                                (((quint32)(unsigned char)inputBuffer[2]) << 8) +
                                ((quint32)(unsigned char)inputBuffer[3]);
                inputBuffer.remove(0, 4);
                messageInProgress = true;
            } else
                return;
        }
        if (inputBuffer.size() < messageLength || messageLength < 0)
            return;

        CommandContainer newCommandContainer;
        try {
            newCommandContainer.ParseFromArray(inputBuffer.data(), messageLength);
        } catch (std::exception &e) {
            qDebug() << "Caught std::exception in" << __FILE__ << __LINE__ <<
#ifdef _MSC_VER // Visual Studio
                __FUNCTION__;
#else
                __PRETTY_FUNCTION__;
#endif
            qDebug() << "Exception:" << e.what();
            qDebug() << "Message coming from:" << getAddress();
            qDebug() << "Message length:" << messageLength;
            qDebug() << "Message content:" << inputBuffer.toHex();
        } catch (...) {
            qDebug() << "Unhandled exception in" << __FILE__ << __LINE__ <<
#ifdef _MSC_VER // Visual Studio
                __FUNCTION__;
#else
                __PRETTY_FUNCTION__;
#endif
            qDebug() << "Message coming from:" << getAddress();
        }

        inputBuffer.remove(0, messageLength);
        messageInProgress = false;

        // dirty hack to make v13 client display the correct error message
        if (handshakeStarted)
            processCommandContainer(newCommandContainer);
        else if (!newCommandContainer.has_cmd_id()) {
            handshakeStarted = true;
            if (!initTcpSession())
                prepareDestroy();
        }
        // end of hack
    } while (!inputBuffer.isEmpty());
}

bool TcpServerSocketInterface::initTcpSession()
{
    if (!initSession())
        return false;

    // limit the number of websocket users based on configuration settings
    bool enforceUserLimit = settingsCache->value("security/enable_max_user_limit", false).toBool();
    if (enforceUserLimit) {
        int userLimit = settingsCache->value("security/max_users_tcp", 500).toInt();
        int playerCount = (server->getTCPUserCount() + 1);
        if (playerCount > userLimit) {
            std::cerr << "Max Tcp Users Limit Reached, please increase the max_users_tcp setting." << std::endl;
            logger->logMessage(QString("Max Tcp Users Limit Reached, please increase the max_users_tcp setting."),
                               this);
            Event_ConnectionClosed event;
            event.set_reason(Event_ConnectionClosed::USER_LIMIT_REACHED);
            SessionEvent *se = prepareSessionEvent(event);
            sendProtocolItem(*se);
            delete se;
            return false;
        }
    }

    return true;
}

WebsocketServerSocketInterface::WebsocketServerSocketInterface(Servatrice *_server,
                                                               Servatrice_DatabaseInterface *_databaseInterface,
                                                               QObject *parent)
    : AbstractServerSocketInterface(_server, _databaseInterface, parent), socket(nullptr)
{
}

WebsocketServerSocketInterface::~WebsocketServerSocketInterface()
{
    logger->logMessage("WebsocketServerSocketInterface destructor", this);

    flushOutputQueue();
}

void WebsocketServerSocketInterface::initConnection(void *_socket)
{
    if (_socket == nullptr) {
        return;
    }
    socket = (QWebSocket *)_socket;
    socket->setParent(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    // https://bugreports.qt.io/browse/QTBUG-70693
    socket->setMaxAllowedIncomingMessageSize(1500000); // 1.5MB
#endif

    address = socket->peerAddress();

    QByteArray websocketIPHeader = settingsCache->value("server/web_socket_ip_header", "").toByteArray();
    if (websocketIPHeader.length() > 0 && socket->request().hasRawHeader(websocketIPHeader)) {
        QString header(socket->request().rawHeader(websocketIPHeader));
        QHostAddress parsed(header);
        if (!parsed.isNull()) {
            address = parsed;
        }
    }

    connect(socket, SIGNAL(binaryMessageReceived(const QByteArray &)), this,
            SLOT(binaryMessageReceived(const QByteArray &)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(catchSocketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), this, SLOT(catchSocketDisconnected()));

    // Add this object to the server's list of connections before it can receive socket events.
    // Otherwise, in case of a socket error, it could be removed from the list before it is added.
    server->addClient(this);

    logger->logMessage(
        QString("Incoming websocket connection: %1 (%2)").arg(address.toString()).arg(socket->peerAddress().toString()),
        this);

    if (!initWebsocketSession())
        prepareDestroy();
}

bool WebsocketServerSocketInterface::initWebsocketSession()
{
    if (!initSession())
        return false;

    // limit the number of websocket users based on configuration settings
    bool enforceUserLimit = settingsCache->value("security/enable_max_user_limit", false).toBool();
    if (enforceUserLimit) {
        int userLimit = settingsCache->value("security/max_users_websocket", 500).toInt();
        int playerCount = (server->getWebSocketUserCount() + 1);
        if (playerCount > userLimit) {
            std::cerr << "Max Websocket Users Limit Reached, please increase the max_users_websocket setting."
                      << std::endl;
            logger->logMessage(
                QString("Max Websocket Users Limit Reached, please increase the max_users_websocket setting."), this);
            Event_ConnectionClosed event;
            event.set_reason(Event_ConnectionClosed::USER_LIMIT_REACHED);
            SessionEvent *se = prepareSessionEvent(event);
            sendProtocolItem(*se);
            delete se;
            return false;
        }
    }

    return true;
}

void WebsocketServerSocketInterface::flushOutputQueue()
{
    QMutexLocker locker(&outputQueueMutex);
    if (outputQueue.isEmpty())
        return;

    qint64 totalBytes = 0;
    while (!outputQueue.isEmpty()) {
        ServerMessage item = outputQueue.takeFirst();
        locker.unlock();

        QByteArray buf;
#if GOOGLE_PROTOBUF_VERSION > 3001000
        unsigned int size = static_cast<unsigned int>(item.ByteSizeLong());
#else
        unsigned int size = static_cast<unsigned int>(item.ByteSize());
#endif
        buf.resize(size);
        item.SerializeToArray(buf.data(), size);
        // In case socket->write() calls catchSocketError(), the mutex must not be locked during this call.
        writeToSocket(buf);

        totalBytes += size;
        locker.relock();
    }
    locker.unlock();
    emit incTxBytes(totalBytes);
    // see above wrt mutex
    flushSocket();
}

void WebsocketServerSocketInterface::binaryMessageReceived(const QByteArray &message)
{
    servatrice->incRxBytes(message.size());

    CommandContainer newCommandContainer;
    try {
        newCommandContainer.ParseFromArray(message.data(), message.size());
    } catch (std::exception &e) {
        qDebug() << "Caught std::exception in" << __FILE__ << __LINE__ <<
#ifdef _MSC_VER // Visual Studio
            __FUNCTION__;
#else
            __PRETTY_FUNCTION__;
#endif
        qDebug() << "Exception:" << e.what();
        qDebug() << "Message coming from:" << getAddress();
        qDebug() << "Message length:" << message.size();
        qDebug() << "Message content:" << message.toHex();
    } catch (...) {
        qDebug() << "Unhandled exception in" << __FILE__ << __LINE__ <<
#ifdef _MSC_VER // Visual Studio
            __FUNCTION__;
#else
            __PRETTY_FUNCTION__;
#endif
        qDebug() << "Message coming from:" << getAddress();
    }

    processCommandContainer(newCommandContainer);
}

bool AbstractServerSocketInterface::isPasswordLongEnough(const int passwordLength)
{
    return passwordLength >= servatrice->getMinPasswordLength();
}
