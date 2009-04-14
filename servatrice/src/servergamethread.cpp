#include "servergamethread.h"
#include "servergame.h"

ServerGameThread::ServerGameThread(int _gameId, const QString _description, const QString _password, const int _maxPlayers, ServerSocket *_creator, QObject *parent)
	: QThread(parent), gameId(_gameId), description(_description), password(_password), maxPlayers(_maxPlayers), creator(_creator), game(0)
{

}

ServerGameThread::~ServerGameThread()
{
	if (game)
		delete game;
}

void ServerGameThread::run()
{
	game = new ServerGame(creator, gameId, description, password, maxPlayers);
	emit gameCreated(game, creator);
	exec();
}
