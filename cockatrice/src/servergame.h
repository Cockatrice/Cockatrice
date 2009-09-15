#ifndef SERVERGAME_H
#define SERVERGAME_H

class ServerGame {
private:
	int gameId;
	QString creator;
	QString description;
	bool hasPassword;
	unsigned char playerCount;
	unsigned char maxPlayers;
	bool spectatorsAllowed;
	unsigned int spectatorsCount;
public:
	ServerGame(int _gameId, const QString &_creator, const QString &_description, bool _hasPassword, unsigned char _playerCount, unsigned char _maxPlayers, bool _spectatorsAllowed, unsigned int _spectatorsCount)
		: gameId(_gameId), creator(_creator), description(_description), hasPassword(_hasPassword), playerCount(_playerCount), maxPlayers(_maxPlayers), spectatorsAllowed(_spectatorsAllowed), spectatorsCount(_spectatorsCount) { }
	int getGameId() const { return gameId; }
	QString getCreator() const { return creator; }
	QString getDescription() const { return description; }
	bool getHasPassword() const { return hasPassword; }
	unsigned char getPlayerCount() const { return playerCount; }
	unsigned char getMaxPlayers() const { return maxPlayers; }
	bool getSpectatorsAllowed() const { return spectatorsAllowed; }
	unsigned int getSpectatorsCount() const { return spectatorsCount; }
};

#endif
