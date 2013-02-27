#ifndef SERVATRICE_DATABASE_INTERFACE_H
#define SERVATRICE_DATABASE_INTERFACE_H

#include <QObject>
#include <QSqlDatabase>

#include "server.h"
#include "server_database_interface.h"

class Servatrice;

class Servatrice_DatabaseInterface : public Server_DatabaseInterface {
	Q_OBJECT
private:
	int instanceId;
	QSqlDatabase sqlDatabase;
	Servatrice *server;
	ServerInfo_User evalUserQueryResult(const QSqlQuery &query, bool complete, bool withId = false);
	bool usernameIsValid(const QString &user);
protected:
	AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr, int &secondsLeft);
public slots:
	void initDatabase(const QSqlDatabase &_sqlDatabase);
public:
	Servatrice_DatabaseInterface(int _instanceId, Servatrice *_server);
	~Servatrice_DatabaseInterface();
	void initDatabase(const QString &type, const QString &hostName, const QString &databaseName, const QString &userName, const QString &password);
	bool openDatabase();
	bool checkSql();
	bool execSqlQuery(QSqlQuery &query);
	const QSqlDatabase &getDatabase() { return sqlDatabase; }

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
	
};

#endif
