#include "servatrice.h"
#include "servatrice_database_interface.h"
#include "passwordhasher.h"
#include "serversocketinterface.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

Servatrice_DatabaseInterface::Servatrice_DatabaseInterface(int _instanceId, Servatrice *_server)
	: instanceId(_instanceId),
	  sqlDatabase(QSqlDatabase()),
	  server(_server)
{
}

void Servatrice_DatabaseInterface::initDatabase(const QSqlDatabase &_sqlDatabase)
{
	sqlDatabase = QSqlDatabase::cloneDatabase(_sqlDatabase, "pool_" + QString::number(instanceId));
	openDatabase();
}

bool Servatrice_DatabaseInterface::openDatabase()
{
	if (sqlDatabase.isOpen())
		sqlDatabase.close();
	
	qDebug() << QString("[pool %1] Opening database...").arg(instanceId);
	if (!sqlDatabase.open()) {
		qCritical() << QString("[pool %1] Error opening database: %2").arg(instanceId).arg(sqlDatabase.lastError().text());
		return false;
	}
	
	return true;
}

bool Servatrice_DatabaseInterface::checkSql()
{
	if (!sqlDatabase.isValid())
		return false;
	
	if (!sqlDatabase.exec("select 1").isActive())
		return openDatabase();
	return true;
}

bool Servatrice_DatabaseInterface::execSqlQuery(QSqlQuery &query)
{
	if (query.exec())
		return true;
	qCritical() << QString("[pool %1] Error executing query: %2").arg(instanceId).arg(query.lastError().text());
	return false;
}

AuthenticationResult Servatrice_DatabaseInterface::checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr, int &banSecondsLeft)
{
	switch (server->getAuthenticationMethod()) {
	case Servatrice::AuthenticationNone: return UnknownUser;
	case Servatrice::AuthenticationSql: {
		if (!checkSql())
			return UnknownUser;
		
		QSqlQuery ipBanQuery(sqlDatabase);
		ipBanQuery.prepare("select time_to_sec(timediff(now(), date_add(b.time_from, interval b.minutes minute))), b.minutes <=> 0, b.visible_reason from " + server->getDbPrefix() + "_bans b where b.time_from = (select max(c.time_from) from " + server->getDbPrefix() + "_bans c where c.ip_address = :address) and b.ip_address = :address2");
		ipBanQuery.bindValue(":address", static_cast<ServerSocketInterface *>(handler)->getPeerAddress().toString());
		ipBanQuery.bindValue(":address2", static_cast<ServerSocketInterface *>(handler)->getPeerAddress().toString());
		if (!execSqlQuery(ipBanQuery)) {
			qDebug("Login denied: SQL error");
			return NotLoggedIn;
		}
		
		if (ipBanQuery.next()) {
			const int secondsLeft = -ipBanQuery.value(0).toInt();
			const bool permanentBan = ipBanQuery.value(1).toInt();
			if ((secondsLeft > 0) || permanentBan) {
				reasonStr = ipBanQuery.value(2).toString();
				banSecondsLeft = permanentBan ? 0 : secondsLeft;
				qDebug("Login denied: banned by address");
				return UserIsBanned;
			}
		}
		
		QSqlQuery nameBanQuery(sqlDatabase);
		nameBanQuery.prepare("select time_to_sec(timediff(now(), date_add(b.time_from, interval b.minutes minute))), b.minutes <=> 0, b.visible_reason from " + server->getDbPrefix() + "_bans b where b.time_from = (select max(c.time_from) from " + server->getDbPrefix() + "_bans c where c.user_name = :name2) and b.user_name = :name1");
		nameBanQuery.bindValue(":name1", user);
		nameBanQuery.bindValue(":name2", user);
		if (!execSqlQuery(nameBanQuery)) {
			qDebug("Login denied: SQL error");
			return NotLoggedIn;
		}
		
		if (nameBanQuery.next()) {
			const int secondsLeft = -nameBanQuery.value(0).toInt();
			const bool permanentBan = nameBanQuery.value(1).toInt();
			if ((secondsLeft > 0) || permanentBan) {
				reasonStr = nameBanQuery.value(2).toString();
				banSecondsLeft = permanentBan ? 0 : secondsLeft;
				qDebug("Login denied: banned by name");
				return UserIsBanned;
			}
		}
		
		QSqlQuery passwordQuery(sqlDatabase);
		passwordQuery.prepare("select password_sha512 from " + server->getDbPrefix() + "_users where name = :name and active = 1");
		passwordQuery.bindValue(":name", user);
		if (!execSqlQuery(passwordQuery)) {
			qDebug("Login denied: SQL error");
			return NotLoggedIn;
		}
		
		if (passwordQuery.next()) {
			const QString correctPassword = passwordQuery.value(0).toString();
			if (correctPassword == PasswordHasher::computeHash(password, correctPassword.left(16))) {
				qDebug("Login accepted: password right");
				return PasswordRight;
			} else {
				qDebug("Login denied: password wrong");
				return NotLoggedIn;
			}
		} else {
			qDebug("Login accepted: unknown user");
			return UnknownUser;
		}
	}
	}
	return UnknownUser;
}

bool Servatrice_DatabaseInterface::userExists(const QString &user)
{
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
		checkSql();
	
		QSqlQuery query;
		query.prepare("select 1 from " + server->getDbPrefix() + "_users where name = :name and active = 1");
		query.bindValue(":name", user);
		if (!execSqlQuery(query))
			return false;
		return query.next();
	}
	return false;
}

int Servatrice_DatabaseInterface::getUserIdInDB(const QString &name)
{
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
		QSqlQuery query;
		query.prepare("select id from " + server->getDbPrefix() + "_users where name = :name and active = 1");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return -1;
		if (!query.next())
			return -1;
		return query.value(0).toInt();
	}
	return -1;
}

bool Servatrice_DatabaseInterface::isInBuddyList(const QString &whoseList, const QString &who)
{
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationNone)
		return false;
	
	if (!checkSql())
		return false;
	
	int id1 = getUserIdInDB(whoseList);
	int id2 = getUserIdInDB(who);
	
	QSqlQuery query;
	query.prepare("select 1 from " + server->getDbPrefix() + "_buddylist where id_user1 = :id_user1 and id_user2 = :id_user2");
	query.bindValue(":id_user1", id1);
	query.bindValue(":id_user2", id2);
	if (!execSqlQuery(query))
		return false;
	return query.next();
}

bool Servatrice_DatabaseInterface::isInIgnoreList(const QString &whoseList, const QString &who)
{
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationNone)
		return false;
	
	if (!checkSql())
		return false;
	
	int id1 = getUserIdInDB(whoseList);
	int id2 = getUserIdInDB(who);
	
	QSqlQuery query;
	query.prepare("select 1 from " + server->getDbPrefix() + "_ignorelist where id_user1 = :id_user1 and id_user2 = :id_user2");
	query.bindValue(":id_user1", id1);
	query.bindValue(":id_user2", id2);
	if (!execSqlQuery(query))
		return false;
	return query.next();
}

ServerInfo_User Servatrice_DatabaseInterface::evalUserQueryResult(const QSqlQuery &query, bool complete, bool withId)
{
	ServerInfo_User result;
	
	if (withId)
		result.set_id(query.value(0).toInt());
	result.set_name(query.value(1).toString().toStdString());
	
	const QString country = query.value(5).toString();
	if (!country.isEmpty())
		result.set_country(country.toStdString());
	
	if (complete) {
		const QByteArray avatarBmp = query.value(6).toByteArray();
		if (avatarBmp.size())
			result.set_avatar_bmp(avatarBmp.data(), avatarBmp.size());
	}
	
	const QString genderStr = query.value(4).toString();
	if (genderStr == "m")
		result.set_gender(ServerInfo_User::Male);
	else if (genderStr == "f")
		result.set_gender(ServerInfo_User::Female);
	
	const int is_admin = query.value(2).toInt();
	int userLevel = ServerInfo_User::IsUser | ServerInfo_User::IsRegistered;
	if (is_admin == 1)
		userLevel |= ServerInfo_User::IsAdmin | ServerInfo_User::IsModerator;
	else if (is_admin == 2)
		userLevel |= ServerInfo_User::IsModerator;
	result.set_user_level(userLevel);
	
	const QString realName = query.value(3).toString();
	if (!realName.isEmpty())
		result.set_real_name(realName.toStdString());
	
	return result;
}

ServerInfo_User Servatrice_DatabaseInterface::getUserData(const QString &name, bool withId)
{
	ServerInfo_User result;
	result.set_name(name.toStdString());
	result.set_user_level(ServerInfo_User::IsUser);
	
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
		if (!checkSql())
			return result;
		
		QSqlQuery query;
		query.prepare("select id, name, admin, realname, gender, country, avatar_bmp from " + server->getDbPrefix() + "_users where name = :name and active = 1");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return result;
		
		if (query.next())
			return evalUserQueryResult(query, true, withId);
		else
			return result;
	} else
		return result;
}

void Servatrice_DatabaseInterface::clearSessionTables()
{
	lockSessionTables();
	QSqlQuery query;
	query.prepare("update " + server->getDbPrefix() + "_sessions set end_time=now() where end_time is null and id_server = :id_server");
	query.bindValue(":id_server", server->getServerId());
	query.exec();
	unlockSessionTables();
}

void Servatrice_DatabaseInterface::lockSessionTables()
{
	QSqlQuery("lock tables " + server->getDbPrefix() + "_sessions write, " + server->getDbPrefix() + "_users read").exec();
}

void Servatrice_DatabaseInterface::unlockSessionTables()
{
	QSqlQuery("unlock tables").exec();
}

bool Servatrice_DatabaseInterface::userSessionExists(const QString &userName)
{
	// Call only after lockSessionTables().
	
	QSqlQuery query;
	query.prepare("select 1 from " + server->getDbPrefix() + "_sessions where user_name = :user_name and end_time is null");
	query.bindValue(":user_name", userName);
	query.exec();
	return query.next();
}

qint64 Servatrice_DatabaseInterface::startSession(const QString &userName, const QString &address)
{
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationNone)
		return -1;
	
	if (!checkSql())
		return -1;
	
	QSqlQuery query;
	query.prepare("insert into " + server->getDbPrefix() + "_sessions (user_name, id_server, ip_address, start_time) values(:user_name, :id_server, :ip_address, NOW())");
	query.bindValue(":user_name", userName);
	query.bindValue(":id_server", server->getServerId());
	query.bindValue(":ip_address", address);
	if (execSqlQuery(query))
		return query.lastInsertId().toInt();
	return -1;
}

void Servatrice_DatabaseInterface::endSession(qint64 sessionId)
{
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationNone)
		return;
	
	if (!checkSql())
		return;
	
	QSqlQuery query;
	query.exec("lock tables " + server->getDbPrefix() + "_sessions write");
	query.prepare("update " + server->getDbPrefix() + "_sessions set end_time=NOW() where id = :id_session");
	query.bindValue(":id_session", sessionId);
	execSqlQuery(query);
	query.exec("unlock tables");
}

QMap<QString, ServerInfo_User> Servatrice_DatabaseInterface::getBuddyList(const QString &name)
{
	QMap<QString, ServerInfo_User> result;
	
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
		checkSql();

		QSqlQuery query;
		query.prepare("select a.id, a.name, a.admin, a.realname, a.gender, a.country from " + server->getDbPrefix() + "_users a left join " + server->getDbPrefix() + "_buddylist b on a.id = b.id_user2 left join " + server->getDbPrefix() + "_users c on b.id_user1 = c.id where c.name = :name");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return result;
		
		while (query.next()) {
			const ServerInfo_User &temp = evalUserQueryResult(query, false);
			result.insert(QString::fromStdString(temp.name()), temp);
		}
	}
	return result;
}

QMap<QString, ServerInfo_User> Servatrice_DatabaseInterface::getIgnoreList(const QString &name)
{
	QMap<QString, ServerInfo_User> result;
	
	if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
		checkSql();

		QSqlQuery query;
		query.prepare("select a.id, a.name, a.admin, a.realname, a.gender, a.country from " + server->getDbPrefix() + "_users a left join " + server->getDbPrefix() + "_ignorelist b on a.id = b.id_user2 left join " + server->getDbPrefix() + "_users c on b.id_user1 = c.id where c.name = :name");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return result;
		
		while (query.next()) {
			ServerInfo_User temp = evalUserQueryResult(query, false);
			result.insert(QString::fromStdString(temp.name()), temp);
		}
	}
	return result;
}

int Servatrice_DatabaseInterface::getNextGameId()
{
	if (!checkSql())
		return Server_DatabaseInterface::getNextGameId();
	
	QSqlQuery query;
	query.prepare("insert into " + dbPrefix + "_games (time_started) values (now())");
	execSqlQuery(query);
	
	return query.lastInsertId().toInt();
}

int Servatrice_DatabaseInterface::getNextReplayId()
{
	if (!checkSql())
		return Server_DatabaseInterface::getNextReplayId();
	
	QSqlQuery query;
	query.prepare("insert into " + dbPrefix + "_replays () values ()");
	execSqlQuery(query);
	
	return query.lastInsertId().toInt();
}

void Servatrice_DatabaseInterface::storeGameInformation(int secondsElapsed, const QSet<QString> &allPlayersEver, const QSet<QString> &allSpectatorsEver, const QList<GameReplay *> &replayList)
{
	const ServerInfo_Game &gameInfo = replayList.first()->game_info();
	
	Server_Room *room = rooms.value(gameInfo.room_id());
	
	Event_ReplayAdded replayEvent;
	ServerInfo_ReplayMatch *replayMatchInfo = replayEvent.mutable_match_info();
	replayMatchInfo->set_game_id(gameInfo.game_id());
	replayMatchInfo->set_room_name(room->getName().toStdString());
	replayMatchInfo->set_time_started(QDateTime::currentDateTime().addSecs(-secondsElapsed).toTime_t());
	replayMatchInfo->set_length(secondsElapsed);
	replayMatchInfo->set_game_name(gameInfo.description());
	
	const QStringList &allGameTypes = room->getGameTypes();
	QStringList gameTypes;
	for (int i = gameInfo.game_types_size() - 1; i >= 0; --i)
		gameTypes.append(allGameTypes[gameInfo.game_types(i)]);
	
	QVariantList gameIds1, playerNames, gameIds2, userIds, replayNames;
	QSetIterator<QString> playerIterator(allPlayersEver);
	while (playerIterator.hasNext()) {
		gameIds1.append(gameInfo.game_id());
		const QString &playerName = playerIterator.next();
		playerNames.append(playerName);
		replayMatchInfo->add_player_names(playerName.toStdString());
	}
	QSet<QString> allUsersInGame = allPlayersEver + allSpectatorsEver;
	QSetIterator<QString> allUsersIterator(allUsersInGame);
	while (allUsersIterator.hasNext()) {
		int id = getUserIdInDB(allUsersIterator.next());
		if (id == -1)
			continue;
		gameIds2.append(gameInfo.game_id());
		userIds.append(id);
		replayNames.append(QString::fromStdString(gameInfo.description()));
	}
	
	QVariantList replayIds, replayGameIds, replayDurations, replayBlobs;
	for (int i = 0; i < replayList.size(); ++i) {
		QByteArray blob;
		const unsigned int size = replayList[i]->ByteSize();
		blob.resize(size);
		replayList[i]->SerializeToArray(blob.data(), size);
		
		replayIds.append(QVariant((qulonglong) replayList[i]->replay_id()));
		replayGameIds.append(gameInfo.game_id());
		replayDurations.append(replayList[i]->duration_seconds());
		replayBlobs.append(blob);
		
		ServerInfo_Replay *replayInfo = replayMatchInfo->add_replay_list();
		replayInfo->set_replay_id(replayList[i]->replay_id());
		replayInfo->set_replay_name(gameInfo.description());
		replayInfo->set_duration(replayList[i]->duration_seconds());
	}
	
	SessionEvent *sessionEvent = Server_ProtocolHandler::prepareSessionEvent(replayEvent);
	allUsersIterator.toFront();
	clientsLock.lockForRead();
	while (allUsersIterator.hasNext()) {
		const QString userName = allUsersIterator.next();
		Server_AbstractUserInterface *userHandler = users.value(userName);
		if (!userHandler)
			userHandler = externalUsers.value(userName);
		if (userHandler)
			userHandler->sendProtocolItem(*sessionEvent);
	}
	clientsLock.unlock();
	delete sessionEvent;
	
	if (!checkSql())
		return;
	
	QSqlQuery query1;
	query1.prepare("update " + dbPrefix + "_games set room_name=:room_name, descr=:descr, creator_name=:creator_name, password=:password, game_types=:game_types, player_count=:player_count, time_finished=now() where id=:id_game");
	query1.bindValue(":room_name", room->getName());
	query1.bindValue(":id_game", gameInfo.game_id());
	query1.bindValue(":descr", QString::fromStdString(gameInfo.description()));
	query1.bindValue(":creator_name", QString::fromStdString(gameInfo.creator_info().name()));
	query1.bindValue(":password", gameInfo.with_password() ? 1 : 0);
	query1.bindValue(":game_types", gameTypes.isEmpty() ? QString("") : gameTypes.join(", "));
	query1.bindValue(":player_count", gameInfo.max_players());
	if (!execSqlQuery(query1))
		return;
	
	QSqlQuery query2;
	query2.prepare("insert into " + dbPrefix + "_games_players (id_game, player_name) values (:id_game, :player_name)");
	query2.bindValue(":id_game", gameIds1);
	query2.bindValue(":player_name", playerNames);
	query2.execBatch();
	
	QSqlQuery replayQuery1;
	replayQuery1.prepare("update " + dbPrefix + "_replays set id_game=:id_game, duration=:duration, replay=:replay where id=:id_replay");
	replayQuery1.bindValue(":id_replay", replayIds);
	replayQuery1.bindValue(":id_game", replayGameIds);
	replayQuery1.bindValue(":duration", replayDurations);
	replayQuery1.bindValue(":replay", replayBlobs);
	replayQuery1.execBatch();
	
	QSqlQuery query3;
	query3.prepare("insert into " + dbPrefix + "_replays_access (id_game, id_player, replay_name) values (:id_game, :id_player, :replay_name)");
	query3.bindValue(":id_game", gameIds2);
	query3.bindValue(":id_player", userIds);
	query3.bindValue(":replay_name", replayNames);
	query3.execBatch();
}

DeckList *Servatrice_DatabaseInterface::getDeckFromDatabase(int deckId, const QString &userName)
{
	checkSql();
	
	QSqlQuery query;
	
	query.prepare("select content from " + dbPrefix + "_decklist_files where id = :id and user = :user");
	query.bindValue(":id", deckId);
	query.bindValue(":user", userName);
	execSqlQuery(query);
	if (!query.next())
		throw Response::RespNameNotFound;
	
	QXmlStreamReader deckReader(query.value(0).toString());
	DeckList *deck = new DeckList;
	deck->loadFromXml(&deckReader);
	
	return deck;
}
