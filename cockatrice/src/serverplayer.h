#ifndef SERVERPLAYER_H
#define SERVERPLAYER_H

#include <QString>

class ServerPlayer {
private:
	int PlayerId;
	QString name;
public:
	ServerPlayer(int _PlayerId, const QString &_name)
		: PlayerId(_PlayerId), name(_name) { }
	int getPlayerId() { return PlayerId; }
	QString getName() { return name; }
};

#endif
