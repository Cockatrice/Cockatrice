#ifndef SERVERPLAYER_H
#define SERVERPLAYER_H

#include <QString>

class ServerPlayer {
private:
	int PlayerId;
	QString name;
	bool local;
public:
	ServerPlayer(int _PlayerId, const QString &_name, bool _local)
		: PlayerId(_PlayerId), name(_name), local(_local) { }
	int getPlayerId() const { return PlayerId; }
	QString getName() const { return name; }
	bool getLocal() const { return local; }
};

#endif
