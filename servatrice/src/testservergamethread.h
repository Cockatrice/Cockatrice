#ifndef TESTSERVERGAMETHREAD_H
#define TESTSERVERGAMETHREAD_H

#include <QThread>
#include <QMutex>
#include "testservergame.h"
#include "testserversocket.h"

class TestServerGame;
class TestServerSocket;

class TestServerGameThread : public QThread {
	Q_OBJECT
signals:
	void gameCreated(TestServerGame *_game, TestServerSocket *creator);
private:
	QString name;
	QString description;
	QString password;
	int maxPlayers;
	TestServerSocket *creator;
	TestServerGame *game;
public:
	TestServerGameThread(const QString _name, const QString _description, const QString _password, const int _maxPlayers, TestServerSocket *_creator, QObject *parent = 0);
	~TestServerGameThread();
	TestServerGame *getGame() { return game; }
	void run();
};

#endif
