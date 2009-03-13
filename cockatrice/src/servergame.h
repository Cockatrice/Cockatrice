#ifndef SERVERGAME_H
#define SERVERGAME_H

class ServerGame {
private:
	QString name;
	QString description;
	bool hasPassword;
	unsigned char playerCount;
	unsigned char maxPlayers;
public:
	ServerGame(const QString &_name, const QString &_description, bool _hasPassword, unsigned char _playerCount, unsigned char _maxPlayers)
		: name(_name), description(_description), hasPassword(_hasPassword), playerCount(_playerCount), maxPlayers(_maxPlayers) { }
	QString getName() { return name; }
	QString getDescription() { return description; }
	bool getHasPassword() { return hasPassword; }
	unsigned char getPlayerCount() { return playerCount; }
	unsigned char getMaxPlayers() { return maxPlayers; }
};

#endif
