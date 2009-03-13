#include "testservergamethread.h"

TestServerGameThread::TestServerGameThread(const QString _name, const QString _description, const QString _password, const int _maxPlayers, TestServerSocket *_creator, QObject *parent)
	: QThread(parent), name(_name), description(_description), password(_password), maxPlayers(_maxPlayers), creator(_creator), game(0)
{

}

TestServerGameThread::~TestServerGameThread()
{
	if (game)
		delete game;
}

void TestServerGameThread::run()
{
	game = new TestServerGame(name, description, password, maxPlayers);
	emit gameCreated(game, creator);
	exec();
}
