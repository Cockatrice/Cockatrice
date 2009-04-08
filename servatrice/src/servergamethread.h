#ifndef SERVERGAMETHREAD_H
#define SERVERGAMETHREAD_H

#include <QThread>
#include <QMutex>

class ServerGame;
class ServerSocket;

class ServerGameThread : public QThread {
	Q_OBJECT
signals:
	void gameCreated(ServerGame *_game, ServerSocket *creator);
private:
	QString name;
	QString description;
	QString password;
	int maxPlayers;
	ServerSocket *creator;
	ServerGame *game;
public:
	ServerGameThread(const QString _name, const QString _description, const QString _password, const int _maxPlayers, ServerSocket *_creator, QObject *parent = 0);
	~ServerGameThread();
	ServerGame *getGame() { return game; }
	void run();
};

#endif
