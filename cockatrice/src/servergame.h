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
public:
	ServerGame(int _gameId, const QString &_creator, const QString &_description, bool _hasPassword, unsigned char _playerCount, unsigned char _maxPlayers)
		: gameId(_gameId), creator(_creator), description(_description), hasPassword(_hasPassword), playerCount(_playerCount), maxPlayers(_maxPlayers) { }
	int getGameId() { return gameId; }
	QString getCreator() { return creator; }
	QString getDescription() { return description; }
	bool getHasPassword() { return hasPassword; }
	unsigned char getPlayerCount() { return playerCount; }
	unsigned char getMaxPlayers() { return maxPlayers; }
};

#endif
