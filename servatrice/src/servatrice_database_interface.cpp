#include "servatrice.h"
#include "servatrice_database_interface.h"
#include "passwordhasher.h"
#include "serversocketinterface.h"
#include "settingscache.h"
#include "decklist.h"
#include "pb/game_replay.pb.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QChar>

Servatrice_DatabaseInterface::Servatrice_DatabaseInterface(int _instanceId, Servatrice *_server)
    : instanceId(_instanceId),
      sqlDatabase(QSqlDatabase()),
      server(_server)
{
}

Servatrice_DatabaseInterface::~Servatrice_DatabaseInterface()
{
    // reset all prepared statements
    qDeleteAll(preparedStatements);
    preparedStatements.clear();

    sqlDatabase.close();
}

void Servatrice_DatabaseInterface::initDatabase(const QSqlDatabase &_sqlDatabase)
{
    if (_sqlDatabase.isValid()) {
        sqlDatabase = QSqlDatabase::cloneDatabase(_sqlDatabase, "pool_" + QString::number(instanceId));
        openDatabase();
    }
}

bool Servatrice_DatabaseInterface::initDatabase(const QString &type, const QString &hostName,
                                                const QString &databaseName, const QString &userName,
                                                const QString &password)
{
    sqlDatabase = QSqlDatabase::addDatabase(type, "main");
    sqlDatabase.setHostName(hostName);
    sqlDatabase.setDatabaseName(databaseName);
    sqlDatabase.setUserName(userName);
    sqlDatabase.setPassword(password);

    return openDatabase();
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

    QSqlQuery *versionQuery = prepareQuery("select version from {prefix}_schema_version limit 1");
    if (!execSqlQuery(versionQuery)) {
        qCritical() << QString("[%1] Error opening database: unable to load database schema version (hint: ensure the cockatrice_schema_version exists)").arg(poolStr);
        return false;
    }

    if (versionQuery->next()) {
        const int dbversion = versionQuery->value(0).toInt();
        const int expectedversion = DATABASE_SCHEMA_VERSION;
        if(dbversion < expectedversion)
        {
            qCritical() << QString("[%1] Error opening database: the database schema version is too old, you need to run the migrations to update it from version %2 to version %3").arg(poolStr).arg(dbversion).arg(expectedversion);
            return false;
        } else if(dbversion > expectedversion) {
            qCritical() << QString("[%1] Error opening database: the database schema version %2 is too new, you need to update servatrice (this servatrice actually uses version %3)").arg(poolStr).arg(dbversion).arg(expectedversion);
            return false;
        }
    } else {
        qCritical() << QString("[%1] Error opening database: unable to load database schema version (hint: ensure the cockatrice_schema_version contains a single record)").arg(poolStr);
        return false;
    }

    // reset all prepared statements
    qDeleteAll(preparedStatements);
    preparedStatements.clear();
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

QSqlQuery * Servatrice_DatabaseInterface::prepareQuery(const QString &queryText)
{
    if(preparedStatements.contains(queryText))
        return preparedStatements.value(queryText);

    QString prefixedQueryText = queryText;
    prefixedQueryText.replace("{prefix}", server->getDbPrefix());
    QSqlQuery * query = new QSqlQuery(sqlDatabase);
    query->prepare(prefixedQueryText);

    preparedStatements.insert(queryText, query);
    return query;
}

bool Servatrice_DatabaseInterface::execSqlQuery(QSqlQuery *query)
{
    if (query->exec())
        return true;
    const QString poolStr = instanceId == -1 ? QString("main") : QString("pool %1").arg(instanceId);
    qCritical() << QString("[%1] Error executing query: %2").arg(poolStr).arg(query->lastError().text());
    return false;
}

bool Servatrice_DatabaseInterface::usernameIsValid(const QString &user, QString & error)
{
    int minNameLength = settingsCache->value("users/minnamelength", 6).toInt();
    if(minNameLength < 1)
        minNameLength = 1;
    int maxNameLength = settingsCache->value("users/maxnamelength", 12).toInt();
    bool allowLowercase = settingsCache->value("users/allowlowercase", true).toBool();
    bool allowUppercase = settingsCache->value("users/allowuppercase", true).toBool();
    bool allowNumerics = settingsCache->value("users/allownumerics", true).toBool();
    bool allowPunctuationPrefix = settingsCache->value("users/allowpunctuationprefix", false).toBool();
    QString allowedPunctuation = settingsCache->value("users/allowedpunctuation", "_").toString();
    error = QString("%1|%2|%3|%4|%5|%6|%7").arg(minNameLength).arg(maxNameLength).arg(allowLowercase).arg(allowUppercase).arg(allowNumerics).arg(allowPunctuationPrefix).arg(allowedPunctuation);

    if (user.length() < minNameLength || user.length() > maxNameLength)
        return false;

    if (!allowPunctuationPrefix && allowedPunctuation.contains(user.at(0)))
        return false;

    QString regEx("[");
    if (allowLowercase)
        regEx.append("a-z");
    if (allowUppercase)
        regEx.append("A-Z");
    if(allowNumerics)
        regEx.append("0-9");
    regEx.append(QRegExp::escape(allowedPunctuation));
    regEx.append("]+");

    static QRegExp re = QRegExp(regEx);
    return re.exactMatch(user);
}

bool Servatrice_DatabaseInterface::registerUser(const QString &userName, const QString &realName, ServerInfo_User_Gender const &gender, const QString &password, const QString &emailAddress, const QString &country, QString &token, bool active)
{
    if (!checkSql())
        return false;

    QString passwordSha512 = PasswordHasher::computeHash(password, PasswordHasher::generateRandomSalt());
    token = active ? QString() : PasswordHasher::generateActivationToken();

    QSqlQuery *query = prepareQuery("insert into {prefix}_users "
            "(name, realname, gender, password_sha512, email, country, registrationDate, active, token) "
            "values "
            "(:userName, :realName, :gender, :password_sha512, :email, :country, UTC_TIMESTAMP(), :active, :token)");
    query->bindValue(":userName", userName);
    query->bindValue(":realName", realName);
    query->bindValue(":gender", getGenderChar(gender));
    query->bindValue(":password_sha512", passwordSha512);
    query->bindValue(":email", emailAddress);
    query->bindValue(":country", country);
    query->bindValue(":active", active ? 1 : 0);
    query->bindValue(":token", token);

    if (!execSqlQuery(query)) {
        qDebug() << "Failed to insert user: " << query->lastError() << " sql: " << query->lastQuery();
        return false;
    }

    return true;
}

bool Servatrice_DatabaseInterface::activateUser(const QString &userName, const QString &token)
{
    if (!checkSql())
        return false;

    QSqlQuery *activateQuery = prepareQuery("select name from {prefix}_users where active=0 and name=:username and token=:token");

    activateQuery->bindValue(":username", userName);
    activateQuery->bindValue(":token", token);
    if (!execSqlQuery(activateQuery)) {
        qDebug() << "Account activation failed: SQL error." << activateQuery->lastError()<< " sql: " << activateQuery->lastQuery();
        return false;
    }

    if (activateQuery->next()) {
        const QString name = activateQuery->value(0).toString();
        // redundant check
        if(name == userName)
        {

            QSqlQuery *query = prepareQuery("update {prefix}_users set active=1 where name = :userName");
            query->bindValue(":userName", userName);

            if (!execSqlQuery(query)) {
                qDebug() << "Failed to activate user: " << query->lastError() << " sql: " << query->lastQuery();
                return false;
            }

            return true;
        }
    }
    return false;
}

QChar Servatrice_DatabaseInterface::getGenderChar(ServerInfo_User_Gender const &gender)
{
    switch (gender) {
        case ServerInfo_User_Gender_GenderUnknown:
            return QChar('u');
        case ServerInfo_User_Gender_Male:
            return QChar('m');
        case ServerInfo_User_Gender_Female:
            return QChar('f');
        default:
            return QChar('u');
    }
}

AuthenticationResult Servatrice_DatabaseInterface::checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, const QString &clientId, QString &reasonStr, int &banSecondsLeft)
{
    switch (server->getAuthenticationMethod()) {
    case Servatrice::AuthenticationNone: return UnknownUser;
    case Servatrice::AuthenticationPassword: {
        QString configPassword = settingsCache->value("authentication/password").toString();
        if (configPassword == password)
            return PasswordRight;

        return NotLoggedIn;
    }
    case Servatrice::AuthenticationSql: {
        if (!checkSql())
            return UnknownUser;

        if (!usernameIsValid(user, reasonStr))
            return UsernameInvalid;

        if (checkUserIsBanned(handler->getAddress(), user, clientId, reasonStr, banSecondsLeft))
            return UserIsBanned;

        QSqlQuery *passwordQuery = prepareQuery("select password_sha512, active from {prefix}_users where name = :name");
        passwordQuery->bindValue(":name", user);
        if (!execSqlQuery(passwordQuery)) {
            qDebug("Login denied: SQL error");
            return NotLoggedIn;
        }

        if (passwordQuery->next()) {
            const QString correctPassword = passwordQuery->value(0).toString();
            const bool userIsActive = passwordQuery->value(1).toBool();
            if(!userIsActive) {
                qDebug("Login denied: user not active");
                return UserIsInactive;
            }
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

bool Servatrice_DatabaseInterface::checkUserIsBanned(const QString &ipAddress, const QString &userName, const QString &clientId, QString &banReason, int &banSecondsRemaining)
{
    if (server->getAuthenticationMethod() != Servatrice::AuthenticationSql)
        return false;

    if (!checkSql()) {
        qDebug("Failed to check if user is banned. Database invalid.");
        return false;
    }

    return
        checkUserIsIpBanned(ipAddress, banReason, banSecondsRemaining) || checkUserIsNameBanned(userName, banReason, banSecondsRemaining) || checkUserIsIdBanned(clientId, banReason, banSecondsRemaining);

}

bool Servatrice_DatabaseInterface::checkUserIsIdBanned(const QString &clientId, QString &banReason, int &banSecondsRemaining)
{
    if (clientId.isEmpty())
        return false;

    QSqlQuery *idBanQuery = prepareQuery(
            "select"
                    " timestampdiff(second, now(), date_add(b.time_from, interval b.minutes minute)),"
                    " b.minutes <=> 0,"
                    " b.visible_reason"
                    " from {prefix}_bans b"
                    " where"
                    " b.time_from = (select max(c.time_from)"
                    " from {prefix}_bans c"
                    " where c.clientid = :id)"
                    " and b.clientid = :id2");

    idBanQuery->bindValue(":id", clientId);
    idBanQuery->bindValue(":id2", clientId);
    if (!execSqlQuery(idBanQuery)) {
        qDebug() << "Id ban check failed: SQL error." << idBanQuery->lastError();
        return false;
    }

    if (idBanQuery->next()) {
        const int secondsLeft = idBanQuery->value(0).toInt();
        const bool permanentBan = idBanQuery->value(1).toInt();
        if ((secondsLeft > 0) || permanentBan) {
            banReason = idBanQuery->value(2).toString();
            banSecondsRemaining = permanentBan ? 0 : secondsLeft;
            qDebug() << "User is banned by client id" << clientId;
            return true;
        }
    }
    return false;
}


bool Servatrice_DatabaseInterface::checkUserIsNameBanned(const QString &userName, QString &banReason, int &banSecondsRemaining)
{
    QSqlQuery *nameBanQuery = prepareQuery("select timestampdiff(second, now(), date_add(b.time_from, interval b.minutes minute)), b.minutes <=> 0, b.visible_reason from {prefix}_bans b where b.time_from = (select max(c.time_from) from {prefix}_bans c where c.user_name = :name2) and b.user_name = :name1");
    nameBanQuery->bindValue(":name1", userName);
    nameBanQuery->bindValue(":name2", userName);
    if (!execSqlQuery(nameBanQuery)) {
        qDebug() << "Name ban check failed: SQL error" << nameBanQuery->lastError();
        return false;
    }

    if (nameBanQuery->next()) {
        const int secondsLeft = nameBanQuery->value(0).toInt();
        const bool permanentBan = nameBanQuery->value(1).toInt();
        if ((secondsLeft > 0) || permanentBan) {
            banReason = nameBanQuery->value(2).toString();
            banSecondsRemaining = permanentBan ? 0 : secondsLeft;
            qDebug() << "Username" << userName << "is banned by name";
            return true;
        }
    }
    return false;
}

bool Servatrice_DatabaseInterface::checkUserIsIpBanned(const QString &ipAddress, QString &banReason, int &banSecondsRemaining)
{
    QSqlQuery *ipBanQuery = prepareQuery(
            "select"
                    " timestampdiff(second, now(), date_add(b.time_from, interval b.minutes minute)),"
                    " b.minutes <=> 0,"
                    " b.visible_reason"
                    " from {prefix}_bans b"
                    " where"
                    " b.time_from = (select max(c.time_from)"
                    " from {prefix}_bans c"
                    " where c.ip_address = :address)"
                    " and b.ip_address = :address2");

    ipBanQuery->bindValue(":address", ipAddress);
    ipBanQuery->bindValue(":address2", ipAddress);
    if (!execSqlQuery(ipBanQuery)) {
        qDebug() << "IP ban check failed: SQL error." << ipBanQuery->lastError();
        return false;
    }

    if (ipBanQuery->next()) {
        const int secondsLeft = ipBanQuery->value(0).toInt();
        const bool permanentBan = ipBanQuery->value(1).toInt();
        if ((secondsLeft > 0) || permanentBan) {
            banReason = ipBanQuery->value(2).toString();
            banSecondsRemaining = permanentBan ? 0 : secondsLeft;
            qDebug() << "User is banned by address" << ipAddress;
            return true;
        }
    }
    return false;
}

bool Servatrice_DatabaseInterface::activeUserExists(const QString &user)
{
    if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
        checkSql();

        QSqlQuery *query = prepareQuery("select 1 from {prefix}_users where name = :name and active = 1");
        query->bindValue(":name", user);
        if (!execSqlQuery(query))
            return false;
        return query->next();
    }
    return false;
}

bool Servatrice_DatabaseInterface::userExists(const QString &user)
{
    if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
        checkSql();

        QSqlQuery *query = prepareQuery("select 1 from {prefix}_users where name = :name");
        query->bindValue(":name", user);
        if (!execSqlQuery(query))
            return false;
        return query->next();
    }
    return false;
}

int Servatrice_DatabaseInterface::getUserIdInDB(const QString &name)
{
    if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
        QSqlQuery *query = prepareQuery("select id from {prefix}_users where name = :name and active = 1");
        query->bindValue(":name", name);
        if (!execSqlQuery(query))
            return -1;
        if (!query->next())
            return -1;
        return query->value(0).toInt();
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

    QSqlQuery *query = prepareQuery("select 1 from {prefix}_buddylist where id_user1 = :id_user1 and id_user2 = :id_user2");
    query->bindValue(":id_user1", id1);
    query->bindValue(":id_user2", id2);
    if (!execSqlQuery(query))
        return false;
    return query->next();
}

bool Servatrice_DatabaseInterface::isInIgnoreList(const QString &whoseList, const QString &who)
{
    if (server->getAuthenticationMethod() == Servatrice::AuthenticationNone)
        return false;

    if (!checkSql())
        return false;

    int id1 = getUserIdInDB(whoseList);
    int id2 = getUserIdInDB(who);

    QSqlQuery *query = prepareQuery("select 1 from {prefix}_ignorelist where id_user1 = :id_user1 and id_user2 = :id_user2");
    query->bindValue(":id_user1", id1);
    query->bindValue(":id_user2", id2);
    if (!execSqlQuery(query))
        return false;
    return query->next();
}

ServerInfo_User Servatrice_DatabaseInterface::evalUserQueryResult(const QSqlQuery *query, bool complete, bool withId)
{
    ServerInfo_User result;

    if (withId)
        result.set_id(query->value(0).toInt());
    result.set_name(query->value(1).toString().toStdString());

    const int is_admin = query->value(2).toInt();
    int userLevel = ServerInfo_User::IsUser | ServerInfo_User::IsRegistered;
    if (is_admin == 1)
        userLevel |= ServerInfo_User::IsAdmin | ServerInfo_User::IsModerator;
    else if (is_admin == 2)
        userLevel |= ServerInfo_User::IsModerator;
    result.set_user_level(userLevel);

    const QString country = query->value(3).toString();
    if (!country.isEmpty())
        result.set_country(country.toStdString());

    if (complete) {
        const QString genderStr = query->value(4).toString();
        if (genderStr == "m")
            result.set_gender(ServerInfo_User::Male);
        else if (genderStr == "f")
            result.set_gender(ServerInfo_User::Female);

        const QString realName = query->value(5).toString();
        if (!realName.isEmpty())
            result.set_real_name(realName.toStdString());

        const QByteArray avatarBmp = query->value(6).toByteArray();
        if (avatarBmp.size())
            result.set_avatar_bmp(avatarBmp.data(), avatarBmp.size());

        const QDateTime regDate = query->value(7).toDateTime();
        if(!regDate.toString(Qt::ISODate).isEmpty()) {
            qint64 accountAgeInSeconds = regDate.secsTo(QDateTime::currentDateTime());
            result.set_accountage_secs(accountAgeInSeconds);
        }

        const QString email = query->value(8).toString();
        if (!email.isEmpty())
            result.set_email(email.toStdString());

        const QString clientid = query->value(9).toString();
        if (!clientid.isEmpty())
            result.set_clientid(clientid.toStdString());
    }
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

        QSqlQuery *query = prepareQuery("select id, name, admin, country, gender, realname, avatar_bmp, registrationDate, email, clientid from {prefix}_users where name = :name and active = 1");
        query->bindValue(":name", name);
        if (!execSqlQuery(query))
            return result;

        if (query->next())
            return evalUserQueryResult(query, true, withId);
        else
            return result;
    } else
        return result;
}

void Servatrice_DatabaseInterface::clearSessionTables()
{
    lockSessionTables();
    QSqlQuery *query = prepareQuery("update {prefix}_sessions set end_time=now() where end_time is null and id_server = :id_server");
    query->bindValue(":id_server", server->getServerId());
    execSqlQuery(query);
    unlockSessionTables();
}

void Servatrice_DatabaseInterface::lockSessionTables()
{
    QSqlQuery *query = prepareQuery("lock tables {prefix}_sessions write, {prefix}_users read");
    execSqlQuery(query);
}

void Servatrice_DatabaseInterface::unlockSessionTables()
{
    QSqlQuery *query = prepareQuery("unlock tables");
    execSqlQuery(query);
}

bool Servatrice_DatabaseInterface::userSessionExists(const QString &userName)
{
    // Call only after lockSessionTables().

    QSqlQuery *query = prepareQuery("select 1 from {prefix}_sessions where user_name = :user_name and id_server = :id_server and end_time is null");
    query->bindValue(":id_server", server->getServerId());
    query->bindValue(":user_name", userName);
    execSqlQuery(query);
    return query->next();
}

qint64 Servatrice_DatabaseInterface::startSession(const QString &userName, const QString &address, const QString &clientId, const QString & connectionType)
{
    if (server->getAuthenticationMethod() == Servatrice::AuthenticationNone)
        return -1;

    if (!checkSql())
        return -1;

    QSqlQuery *query = prepareQuery("insert into {prefix}_sessions (user_name, id_server, ip_address, start_time, clientid, connection_type) values(:user_name, :id_server, :ip_address, NOW(), :client_id, :connection_type)");
    query->bindValue(":user_name", userName);
    query->bindValue(":id_server", server->getServerId());
    query->bindValue(":ip_address", address);
    query->bindValue(":client_id", clientId);
    query->bindValue(":connection_type", connectionType);
    if (execSqlQuery(query))
        return query->lastInsertId().toInt();
    return -1;
}

void Servatrice_DatabaseInterface::endSession(qint64 sessionId)
{
    if (server->getAuthenticationMethod() == Servatrice::AuthenticationNone)
        return;

    if (!checkSql())
        return;

    QSqlQuery *query = prepareQuery("lock tables {prefix}_sessions write");
    execSqlQuery(query);

    query = prepareQuery("update {prefix}_sessions set end_time=NOW() where id = :id_session");
    query->bindValue(":id_session", sessionId);
    execSqlQuery(query);

    query = prepareQuery("unlock tables");
    execSqlQuery(query);
}

QMap<QString, ServerInfo_User> Servatrice_DatabaseInterface::getBuddyList(const QString &name)
{
    QMap<QString, ServerInfo_User> result;

    if (server->getAuthenticationMethod() == Servatrice::AuthenticationSql) {
        checkSql();

        QSqlQuery *query = prepareQuery("select a.id, a.name, a.admin, a.country from {prefix}_users a left join {prefix}_buddylist b on a.id = b.id_user2 left join {prefix}_users c on b.id_user1 = c.id where c.name = :name");
        query->bindValue(":name", name);
        if (!execSqlQuery(query))
            return result;

        while (query->next()) {
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

        QSqlQuery *query = prepareQuery("select a.id, a.name, a.admin, a.country from {prefix}_users a left join {prefix}_ignorelist b on a.id = b.id_user2 left join {prefix}_users c on b.id_user1 = c.id where c.name = :name");
        query->bindValue(":name", name);
        if (!execSqlQuery(query))
            return result;

        while (query->next()) {
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

    QSqlQuery *query = prepareQuery("insert into {prefix}_games (time_started) values (now())");
    execSqlQuery(query);

    return query->lastInsertId().toInt();
}

int Servatrice_DatabaseInterface::getNextReplayId()
{
    if (!checkSql())
        return -1;

    QSqlQuery *query = prepareQuery("insert into {prefix}_replays (id_game) values (NULL)");
    execSqlQuery(query);

    return query->lastInsertId().toInt();
}

void Servatrice_DatabaseInterface::storeGameInformation(const QString &roomName, const QStringList &roomGameTypes, const ServerInfo_Game &gameInfo, const QSet<QString> &allPlayersEver, const QSet<QString> &allSpectatorsEver, const QList<GameReplay *> &replayList)
{
    if (!checkSql())
        return;

    if (!settingsCache->value("game/store_replays", 1).toBool() )
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
        QSqlQuery *query = prepareQuery("update {prefix}_games set room_name=:room_name, descr=:descr, creator_name=:creator_name, password=:password, game_types=:game_types, player_count=:player_count, time_finished=now() where id=:id_game");
        query->bindValue(":room_name", roomName);
        query->bindValue(":id_game", gameInfo.game_id());
        query->bindValue(":descr", QString::fromStdString(gameInfo.description()));
        query->bindValue(":creator_name", QString::fromStdString(gameInfo.creator_info().name()));
        query->bindValue(":password", gameInfo.with_password() ? 1 : 0);
        query->bindValue(":game_types", roomGameTypes.isEmpty() ? QString("") : roomGameTypes.join(", "));
        query->bindValue(":player_count", gameInfo.max_players());
        if (!execSqlQuery(query))
            return;
    }
    {
        QSqlQuery *query = prepareQuery("insert into {prefix}_games_players (id_game, player_name) values (:id_game, :player_name)");
        query->bindValue(":id_game", gameIds1);
        query->bindValue(":player_name", playerNames);
        query->execBatch();
    }
    {
        QSqlQuery *query = prepareQuery("update {prefix}_replays set id_game=:id_game, duration=:duration, replay=:replay where id=:id_replay");
        query->bindValue(":id_replay", replayIds);
        query->bindValue(":id_game", replayGameIds);
        query->bindValue(":duration", replayDurations);
        query->bindValue(":replay", replayBlobs);
        query->execBatch();
    }
    {
        QSqlQuery *query = prepareQuery("insert into {prefix}_replays_access (id_game, id_player, replay_name) values (:id_game, :id_player, :replay_name)");
        query->bindValue(":id_game", gameIds2);
        query->bindValue(":id_player", userIds);
        query->bindValue(":replay_name", replayNames);
        query->execBatch();
    }
}

DeckList *Servatrice_DatabaseInterface::getDeckFromDatabase(int deckId, int userId)
{
    checkSql();

    QSqlQuery *query = prepareQuery("select content from {prefix}_decklist_files where id = :id and id_user = :id_user");
    query->bindValue(":id", deckId);
    query->bindValue(":id_user", userId);
    execSqlQuery(query);
    if (!query->next())
        throw Response::RespNameNotFound;

    DeckList *deck = new DeckList;
    deck->loadFromString_Native(query->value(0).toString());

    return deck;
}

void Servatrice_DatabaseInterface::logMessage(const int senderId, const QString &senderName, const QString &senderIp, const QString &logMessage, LogMessage_TargetType targetType, const int targetId, const QString &targetName)
{
    QString targetTypeString;
    switch(targetType)
    {
        case MessageTargetRoom:
            if(!settingsCache->value("logging/log_user_msg_room", 0).toBool())
                return;
            targetTypeString = "room";
            break;
        case MessageTargetGame:
            if(!settingsCache->value("logging/log_user_msg_game", 0).toBool())
                return;
            targetTypeString = "game";
            break;
        case MessageTargetChat:
            if(!settingsCache->value("logging/log_user_msg_chat", 0).toBool())
                return;
            targetTypeString = "chat";
            break;
        case MessageTargetIslRoom:
            if(!settingsCache->value("logging/log_user_msg_isl", 0).toBool())
                return;
            targetTypeString = "room";
            break;
        default:
            return;
    }

    QSqlQuery *query = prepareQuery("insert into {prefix}_log (log_time, sender_id, sender_name, sender_ip, log_message, target_type, target_id, target_name) values (now(), :sender_id, :sender_name, :sender_ip, :log_message, :target_type, :target_id, :target_name)");
    query->bindValue(":sender_id", senderId < 1 ? QVariant() : senderId);
    query->bindValue(":sender_name", senderName);
    query->bindValue(":sender_ip", senderIp);
    query->bindValue(":log_message", logMessage);
    query->bindValue(":target_type", targetTypeString);
    query->bindValue(":target_id", (targetType == MessageTargetChat && targetId < 1) ? QVariant() : targetId);
    query->bindValue(":target_name", targetName);
    execSqlQuery(query);
}

bool Servatrice_DatabaseInterface::changeUserPassword(const QString &user, const QString &oldPassword, const QString &newPassword)
{
    if(server->getAuthenticationMethod() != Servatrice::AuthenticationSql)
        return true;

    if (!checkSql())
        return true;

    QString error;
    if (!usernameIsValid(user, error))
        return true;

    QSqlQuery *passwordQuery = prepareQuery("select password_sha512 from {prefix}_users where name = :name");
    passwordQuery->bindValue(":name", user);
    if (!execSqlQuery(passwordQuery)) {
        qDebug("Change password denied: SQL error");
        return true;
    }

    if (!passwordQuery->next())
        return true;

    const QString correctPassword = passwordQuery->value(0).toString();
    if (correctPassword != PasswordHasher::computeHash(oldPassword, correctPassword.left(16)))
        return true;

    QString passwordSha512 = PasswordHasher::computeHash(newPassword, PasswordHasher::generateRandomSalt());

    passwordQuery = prepareQuery("update {prefix}_users set password_sha512=:password where name = :name");
    passwordQuery->bindValue(":password", passwordSha512);
    passwordQuery->bindValue(":name", user);
    if (!execSqlQuery(passwordQuery)) {
        qDebug("Change password denied: SQL error");
        return true;
    }
    return false;
}

int Servatrice_DatabaseInterface::getActiveUserCount(QString connectionType)
{
    int userCount = 0;

    if (!checkSql())
        return userCount;

    QString text = "select count(*) from {prefix}_sessions where id_server = :serverid AND end_time is NULL";
    if(!connectionType.isEmpty())
        text +=" AND connection_type = :connection_type";
    QSqlQuery *query = prepareQuery(text);

    query->bindValue(":serverid", server->getServerId());
    if(!connectionType.isEmpty())
        query->bindValue(":connection_type", connectionType);

    if (!execSqlQuery(query))
        return userCount;

    if (query->next())
        userCount = query->value(0).toInt();

    return userCount;
}

void Servatrice_DatabaseInterface::updateUsersClientID(const QString &userName, const QString &userClientID)
{

    if (!checkSql())
        return;

    QSqlQuery *query = prepareQuery("update {prefix}_users set clientid = :clientid where name = :username");
    query->bindValue(":clientid", userClientID);
    query->bindValue(":username", userName);
    execSqlQuery(query);

}

void Servatrice_DatabaseInterface::updateUsersLastLoginData(const QString &userName, const QString &clientVersion) {

    if (!checkSql())
        return;

    int usersID;

    QSqlQuery *query = prepareQuery("select id from {prefix}_users where name = :user_name");
    query->bindValue(":user_name", userName);
    if (!execSqlQuery(query)) {
        qDebug("Failed to locate user id when updating users last login data: SQL Error");
        return;
    }

    if (query->next()) {
        usersID = query->value(0).toInt();
    }

    if (usersID) {
        int userCount;
        query = prepareQuery("select count(id) from {prefix}_user_analytics where id = :user_id");
        query->bindValue(":user_id", usersID);
        if (!execSqlQuery(query))
            return;

        if (query->next()) {
            userCount = query->value(0).toInt();
        }

        if (!userCount) {
            query = prepareQuery("insert into {prefix}_user_analytics (id,client_ver,last_login) values (:user_id,:client_ver,NOW())");
            query->bindValue(":user_id", usersID);
            query->bindValue(":client_ver", clientVersion);
            execSqlQuery(query);
        } else {
            query = prepareQuery("update {prefix}_user_analytics set last_login = NOW(), client_ver = :client_ver where id = :user_id");
            query->bindValue(":client_ver", clientVersion);
            query->bindValue(":user_id", usersID);
            execSqlQuery(query);
        }
    }
}

QList<ServerInfo_Ban> Servatrice_DatabaseInterface::getUserBanHistory(const QString userName)
{
    QList<ServerInfo_Ban> results;
    ServerInfo_Ban banDetails;

    if (!checkSql())
        return results;

    QSqlQuery *query = prepareQuery("SELECT A.id_admin, A.time_from, A.minutes, A.reason, A.visible_reason, B.name AS name_admin FROM {prefix}_bans A LEFT JOIN {prefix}_users B ON A.id_admin=B.id WHERE A.user_name = :user_name");
    query->bindValue(":user_name", userName);

    if (!execSqlQuery(query)) {
        qDebug("Failed to collect ban history information: SQL Error");
        return results;
    }

    while (query->next()){
        banDetails.set_admin_id(QString(query->value(0).toString()).toStdString());
        banDetails.set_admin_name(QString(query->value(5).toString()).toStdString());
        banDetails.set_ban_time(QString(query->value(1).toString()).toStdString());
        banDetails.set_ban_length(QString(query->value(2).toString()).toStdString());
        banDetails.set_ban_reason(QString(query->value(3).toString()).toStdString());
        banDetails.set_visible_reason(QString(query->value(4).toString()).toStdString());
        results << banDetails;
    }

    return results;
}

bool Servatrice_DatabaseInterface::addWarning(const QString userName, const QString adminName, const QString warningReason, const QString clientID)
{
    if (!checkSql())
        return false;

    int userID = getUserIdInDB(userName);
    QSqlQuery *query = prepareQuery("insert into {prefix}_warnings (user_id,user_name,mod_name,reason,time_of,clientid) values (:user_id,:user_name,:mod_name,:warn_reason,NOW(),:client_id)");
    query->bindValue(":user_id", userID);
    query->bindValue(":user_name", userName);
    query->bindValue(":mod_name", adminName);
    query->bindValue(":warn_reason", warningReason);
    query->bindValue(":client_id", clientID);
    if (!execSqlQuery(query)) {
        qDebug("Failed to collect create warning history information: SQL Error");
        return false;
    }

    return true;
}

QList<ServerInfo_Warning> Servatrice_DatabaseInterface::getUserWarnHistory(const QString userName)
{
    QList<ServerInfo_Warning> results;
    ServerInfo_Warning warnDetails;

    if (!checkSql())
        return results;

    int userID = getUserIdInDB(userName);
    QSqlQuery *query = prepareQuery("SELECT user_name, mod_name, reason, time_of FROM {prefix}_warnings WHERE user_id = :user_id");
    query->bindValue(":user_id", userID);

    if (!execSqlQuery(query)) {
        qDebug("Failed to collect warning history information: SQL Error");
        return results;
    }

    while (query->next()){
        warnDetails.set_user_name(QString(query->value(0).toString()).toStdString());
        warnDetails.set_admin_name(QString(query->value(1).toString()).toStdString());
        warnDetails.set_reason(QString(query->value(2).toString()).toStdString());
        warnDetails.set_time_of(QString(query->value(3).toString()).toStdString());
        results << warnDetails;
    }

    return results;
}

QList<ServerInfo_ChatMessage> Servatrice_DatabaseInterface::getMessageLogHistory(const QString &user, const QString &ipaddress, const QString &gamename, const QString &gameid, const QString &message, bool &chat, bool &game, bool &room, int &range, int &maxresults)
{

    QList<ServerInfo_ChatMessage> results;
    ServerInfo_ChatMessage chatMessage;

    if (!checkSql())
        return results;

    // BUILD QUERY STRING BASED ON PASSED IN VALUES
    QString queryString = "SELECT * FROM {prefix}_log WHERE `sender_ip` IS NOT NULL";
    if (!user.isEmpty())
        queryString.append(" AND (`sender_name` = :user_name OR `target_name` = :user_name)");

    if (!ipaddress.isEmpty())
        queryString.append(" AND `sender_ip` = :ip_to_find");

    if (!gameid.isEmpty())
        queryString.append(" AND (`target_id` = :game_id AND `target_type` = 'game')");

    if (!gamename.isEmpty())
        queryString.append(" AND (`target_name` = :game_name AND `target_type` = 'game')");

    if (!message.isEmpty())
        queryString.append(" AND `log_message` LIKE :log_message");

    if (chat || game || room) {
        queryString.append(" AND (");

        if (chat)
            queryString.append("`target_type` = 'chat'");

        if (game) {
            if (chat)
                queryString.append(" OR `target_type` = 'game'");
            else
                queryString.append("`target_type` = 'game'");
        }

        if (room) {
            if (game || chat)
                queryString.append(" OR `target_type` = 'room'");
            else
                queryString.append("`target_type` = 'room'");
        }
        queryString.append(")");
    }

    if (range)
        queryString.append(" AND log_time >= DATE_SUB(now(), INTERVAL :range_time HOUR)");

    if (maxresults)
        queryString.append(" LIMIT :limit_size");

    QSqlQuery *query = prepareQuery(queryString);
    if (!user.isEmpty()) { query->bindValue(":user_name", user); }
    if (!ipaddress.isEmpty()) { query->bindValue(":ip_to_find", ipaddress); }
    if (!gameid.isEmpty()) { query->bindValue(":game_id", gameid); }
    if (!gamename.isEmpty()) { query->bindValue(":game_name", gamename); }
    if (!message.isEmpty()) { query->bindValue(":log_message", message); }
    if (range) { query->bindValue(":range_time", range); }
    if (maxresults) { query->bindValue(":limit_size", maxresults); }

    if (!execSqlQuery(query)) {
        qDebug("Failed to collect log history information: SQL Error");
        return results;
    }

    while (query->next()) {
        chatMessage.set_time(QString(query->value(0).toString()).toStdString());
        chatMessage.set_sender_id(QString(query->value(1).toString()).toStdString());
        chatMessage.set_sender_name(QString(query->value(2).toString()).toStdString());
        chatMessage.set_sender_ip(QString(query->value(3).toString()).toStdString());
        chatMessage.set_message(QString(query->value(4).toString()).toStdString());
        chatMessage.set_target_type(QString(query->value(5).toString()).toStdString());
        chatMessage.set_target_id(QString(query->value(6).toString()).toStdString());
        chatMessage.set_target_name(QString(query->value(7).toString()).toStdString());
        results << chatMessage;
    }

    return results;
}
