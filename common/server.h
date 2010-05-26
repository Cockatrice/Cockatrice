#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QStringList>
#include <QMap>

class Server_Game;
class Server_ChatChannel;
class Server_ProtocolHandler;

enum AuthenticationResult { PasswordWrong = 0, PasswordRight = 1, UnknownUser = 2 };

class Server : public QObject
{
	Q_OBJECT
private slots:
	void gameClosing();
	void broadcastChannelUpdate();
public:
	Server(QObject *parent = 0);
	~Server();
	virtual AuthenticationResult checkUserPassword(const QString &user, const QString &password) = 0;
	QList<Server_Game *> getGames() const { return games.values(); }
	Server_Game *getGame(int gameId) const;
	const QMap<QString, Server_ChatChannel *> &getChatChannels() { return chatChannels; }
	void broadcastGameListUpdate(Server_Game *game);
	
	void addClient(Server_ProtocolHandler *player);
	void removeClient(Server_ProtocolHandler *player);
	void closeOldSession(const QString &playerName);
	virtual QString getLoginMessage() const = 0;
	Server_Game *createGame(const QString &description, const QString &password, int maxPlayers, bool spectatorsAllowed, bool spectatorsNeedPassword, bool spectatorsCanTalk, bool spectatorsSeeEverything, Server_ProtocolHandler *creator);
	
	virtual int getMaxGameInactivityTime() const = 0;
	virtual int getMaxPlayerInactivityTime() const = 0;
private:
	QMap<int, Server_Game *> games;
	QList<Server_ProtocolHandler *> clients;
	QMap<QString, Server_ChatChannel *> chatChannels;
protected:
	int nextGameId;
	void addChatChannel(Server_ChatChannel *newChannel);
};

#endif
