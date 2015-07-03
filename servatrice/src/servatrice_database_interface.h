#ifndef SERVATRICE_DATABASE_INTERFACE_H
#define SERVATRICE_DATABASE_INTERFACE_H

#include <QObject>
#include <QSqlDatabase>
#include <QHash>
#include <QChar>

#include "server.h"
#include "server_database_interface.h"

#define DATABASE_SCHEMA_VERSION 1

class Servatrice;

class Servatrice_DatabaseInterface : public Server_DatabaseInterface {
	Q_OBJECT
private:
	int instanceId;
	QSqlDatabase sqlDatabase;
	QHash<QString, QSqlQuery *> preparedStatements;
	Servatrice *server;
	ServerInfo_User evalUserQueryResult(const QSqlQuery *query, bool complete, bool withId = false);
	/** Must be called after checkSql and server is known to be in auth mode. */
	bool checkUserIsIpBanned(const QString &ipAddress, QString &banReason, int &banSecondsRemaining);
	/** Must be called after checkSql and server is known to be in auth mode. */
	bool checkUserIsNameBanned(QString const &userName, QString &banReason, int &banSecondsRemaining);
	QChar getGenderChar(ServerInfo_User_Gender const &gender);
protected:
	AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr, int &secondsLeft);
public slots:
	void initDatabase(const QSqlDatabase &_sqlDatabase);
public:
	Servatrice_DatabaseInterface(int _instanceId, Servatrice *_server);
	~Servatrice_DatabaseInterface();
	bool initDatabase(const QString &type, const QString &hostName, const QString &databaseName,
					  const QString &userName, const QString &password);
	bool openDatabase();
	bool checkSql();
	QSqlQuery * prepareQuery(const QString &queryText);
	bool execSqlQuery(QSqlQuery *query);
	const QSqlDatabase &getDatabase() { return sqlDatabase; }

	bool activeUserExists(const QString &user);
	bool userExists(const QString &user);
	int getUserIdInDB(const QString &name);
	QMap<QString, ServerInfo_User> getBuddyList(const QString &name);
	QMap<QString, ServerInfo_User> getIgnoreList(const QString &name);
	bool isInBuddyList(const QString &whoseList, const QString &who);
	bool isInIgnoreList(const QString &whoseList, const QString &who);
	ServerInfo_User getUserData(const QString &name, bool withId = false);
	void storeGameInformation(const QString &roomName, const QStringList &roomGameTypes, const ServerInfo_Game &gameInfo, const QSet<QString> &allPlayersEver, const QSet<QString> &allSpectatorsEver, const QList<GameReplay *> &replayList);
	DeckList *getDeckFromDatabase(int deckId, int userId);
	
	int getNextGameId();
	int getNextReplayId();
	
	qint64 startSession(const QString &userName, const QString &address);
	void endSession(qint64 sessionId);
	
	void clearSessionTables();
	void lockSessionTables();
	void unlockSessionTables();
	bool userSessionExists(const QString &userName);
	bool usernameIsValid(const QString &user, QString & error);
	bool checkUserIsBanned(const QString &ipAddress, const QString &userName, QString &banReason, int &banSecondsRemaining);

	bool getRequireRegistration();
	bool registerUser(const QString &userName, const QString &realName, ServerInfo_User_Gender const &gender, const QString &password, const QString &emailAddress, const QString &country, QString &token, bool active = false);
	bool activateUser(const QString &userName, const QString &token);

    void logMessage(const int senderId, const QString &senderName, const QString &senderIp, const QString &logMessage, LogMessage_TargetType targetType, const int targetId, const QString &targetName);
};

#endif
