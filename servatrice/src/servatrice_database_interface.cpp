#include "servatrice.h"
#include "servatrice_database_interface.h"
#include "passwordhasher.h"
#include "serversocketinterface.h"
#include "decklist.h"
#include "pb/game_replay.pb.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

Servatrice_DatabaseInterface::Servatrice_DatabaseInterface(int _instanceId, Servatrice *_server)
	: instanceId(_instanceId),
	  sqlDatabase(QSqlDatabase()),
	  server(_server)
{
}

Servatrice_DatabaseInterface::~Servatrice_DatabaseInterface()
{
	sqlDatabase.close();
}

void Servatrice_DatabaseInterface::initDatabase(const QSqlDatabase &_sqlDatabase)
{
	if (_sqlDatabase.isValid()) {
		sqlDatabase = QSqlDatabase::cloneDatabase(_sqlDatabase, "pool_" + QString::number(instanceId));
		openDatabase();
	}
}

void Servatrice_DatabaseInterface::initDatabase(const QString &type, const QString &hostName, const QString &databaseName, const QString &userName, const QString &password)
{
	sqlDatabase = QSqlDatabase::addDatabase(type, "main");
	sqlDatabase.setHostName(hostName);
	sqlDatabase.setDatabaseName(databaseName);
	sqlDatabase.setUserName(userName);
	sqlDatabase.setPassword(password);
	
	openDatabase();
}

bool Servatrice_DatabaseInterface::openDatabase()
{
	if (sqlDatabase.isOpen())
		sqlDatabase.close();
	
	const QString poolStr = instanceId == -1 ? QString("main") : QString("pool %1").arg(instanceId);
	qDebug() << QString("[%1] Opening database...").arg(poolStr);
	if (!sqlDatabase.open()) {
		qCritical() << QString("[%1] Error opening database: %2").arg(poolStr).arg(sqlDatabase.lastError().text());
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
	const QString poolStr = instanceId == -1 ? QString("main") : QString("pool %1").arg(instanceId);
	qCritical() << QString("[%1] Error executing query: %2").arg(poolStr).arg(query.lastError().text());
	return false;
}

bool Servatrice_DatabaseInterface::usernameIsValid(const QString &user)
{
	QString result;
	result.reserve(user.size());
	foreach (const QChar& c, user) {
		switch (c.category()) {
		// TODO: Figure out exactly which categories are OK and not
		case QChar::Other_Control: break;
		default: result += c;
		}
	}
	result = result.trimmed();
	return (result.size() > 0);
}

AuthenticationResult Servatrice_DatabaseInterface::checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr, int &banSecondsLeft)
{
	switch (server->getAuthenticationMethod()) {
	case Servatrice::AuthenticationNone: return UnknownUser;
	case Servatrice::AuthenticationSql: {
		if (!checkSql())
			return UnknownUser;

		if (!usernameIsValid(user))
			return UsernameInvalid;
		
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
	
		QSqlQuery query(sqlDatabase);
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
		QSqlQuery query(sqlDatabase);
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
	
	QSqlQuery query(sqlDatabase);
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
	
	QSqlQuery query(sqlDatabase);
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
		
		QSqlQuery query(sqlDatabase);
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
	QSqlQuery query(sqlDatabase);
	query.prepare("update " + server->getDbPrefix() + "_sessions set end_time=now() where end_time is null and id_server = :id_server");
	query.bindValue(":id_server", server->getServerId());
	query.exec();
	unlockSessionTables();
}

void Servatrice_DatabaseInterface::lockSessionTables()
{
	QSqlQuery("lock tables " + server->getDbPrefix() + "_sessions write, " + server->getDbPrefix() + "_users read", sqlDatabase).exec();
}

void Servatrice_DatabaseInterface::unlockSessionTables()
{
	QSqlQuery("unlock tables", sqlDatabase).exec();
}

bool Servatrice_DatabaseInterface::userSessionExists(const QString &userName)
{
	// Call only after lockSessionTables().
	
	QSqlQuery query(sqlDatabase);
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
	
	QSqlQuery query(sqlDatabase);
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
	
	QSqlQuery query(sqlDatabase);
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

		QSqlQuery query(sqlDatabase);
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

		QSqlQuery query(sqlDatabase);
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
	if (!sqlDatabase.isValid())
		return server->getNextLocalGameId();
	
	if (!checkSql())
		return -1;
	
	QSqlQuery query(sqlDatabase);
	query.prepare("insert into " + server->getDbPrefix() + "_games (time_started) values (now())");
	execSqlQuery(query);
	
	return query.lastInsertId().toInt();
}

int Servatrice_DatabaseInterface::getNextReplayId()
{
	if (!checkSql())
		return -1;
	
	QSqlQuery query(sqlDatabase);
	query.prepare("insert into " + server->getDbPrefix() + "_replays () values ()");
	execSqlQuery(query);
	
	return query.lastInsertId().toInt();
}

void Servatrice_DatabaseInterface::storeGameInformation(const QString &roomName, const QStringList &roomGameTypes, const ServerInfo_Game &gameInfo, const QSet<QString> &allPlayersEver, const QSet<QString> &allSpectatorsEver, const QList<GameReplay *> &replayList)
{
	if (!checkSql())
		return;
	
	QVariantList gameIds1, playerNames, gameIds2, userIds, replayNames;
	QSetIterator<QString> playerIterator(allPlayersEver);
	while (playerIterator.hasNext()) {
		gameIds1.append(gameInfo.game_id());
		const QString &playerName = playerIterator.next();
		playerNames.append(playerName);
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
	}
	
	{
		QSqlQuery query(sqlDatabase);
		query.prepare("update " + server->getDbPrefix() + "_games set room_name=:room_name, descr=:descr, creator_name=:creator_name, password=:password, game_types=:game_types, player_count=:player_count, time_finished=now() where id=:id_game");
		query.bindValue(":room_name", roomName);
		query.bindValue(":id_game", gameInfo.game_id());
		query.bindValue(":descr", QString::fromStdString(gameInfo.description()));
		query.bindValue(":creator_name", QString::fromStdString(gameInfo.creator_info().name()));
		query.bindValue(":password", gameInfo.with_password() ? 1 : 0);
		query.bindValue(":game_types", roomGameTypes.isEmpty() ? QString("") : roomGameTypes.join(", "));
		query.bindValue(":player_count", gameInfo.max_players());
		if (!execSqlQuery(query))
			return;
	}
	{
		QSqlQuery query(sqlDatabase);
		query.prepare("insert into " + server->getDbPrefix() + "_games_players (id_game, player_name) values (:id_game, :player_name)");
		query.bindValue(":id_game", gameIds1);
		query.bindValue(":player_name", playerNames);
		query.execBatch();
	}
	{
		QSqlQuery query(sqlDatabase);
		query.prepare("update " + server->getDbPrefix() + "_replays set id_game=:id_game, duration=:duration, replay=:replay where id=:id_replay");
		query.bindValue(":id_replay", replayIds);
		query.bindValue(":id_game", replayGameIds);
		query.bindValue(":duration", replayDurations);
		query.bindValue(":replay", replayBlobs);
		query.execBatch();
	}
	{
		QSqlQuery query(sqlDatabase);
		query.prepare("insert into " + server->getDbPrefix() + "_replays_access (id_game, id_player, replay_name) values (:id_game, :id_player, :replay_name)");
		query.bindValue(":id_game", gameIds2);
		query.bindValue(":id_player", userIds);
		query.bindValue(":replay_name", replayNames);
		query.execBatch();
	}
}

DeckList *Servatrice_DatabaseInterface::getDeckFromDatabase(int deckId, int userId)
{
	checkSql();
	
	QSqlQuery query(sqlDatabase);
	
	query.prepare("select content from " + server->getDbPrefix() + "_decklist_files where id = :id and id_user = :id_user");
	query.bindValue(":id", deckId);
	query.bindValue(":id_user", userId);
	execSqlQuery(query);
	if (!query.next())
		throw Response::RespNameNotFound;
	
	DeckList *deck = new DeckList;
	deck->loadFromString_Native(query.value(0).toString());
	
	return deck;
}
