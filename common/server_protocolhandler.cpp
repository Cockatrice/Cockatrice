#include <QDebug>
#include "server_protocolhandler.h"
#include "protocol.h"
#include "protocol_items.h"

Server_ProtocolHandler::Server_ProtocolHandler(Server *_server, QObject *parent)
	: QObject(parent), server(_server), authState(PasswordWrong), acceptsGameListChanges(false)
{
}

Server_ProtocolHandler::~Server_ProtocolHandler()
{
}

void Server_ProtocolHandler::processCommand(Command *command)
{
	ChatCommand *chatCommand = qobject_cast<ChatCommand *>(command);
	GameCommand *gameCommand = qobject_cast<GameCommand *>(command);
	if (chatCommand) {
		qDebug() << "received ChatCommand: channel =" << chatCommand->getChannel();
	} else if (gameCommand) {
		qDebug() << "received GameCommand: game =" << gameCommand->getGameId();
	} else {
		qDebug() << "received generic Command";
	}
}

QPair<Server_Game *, Server_Player *> Server_ProtocolHandler::getGame(int gameId) const
{
	if (games.contains(gameId))
		return games.value(gameId);
	return QPair<Server_Game *, Server_Player *>(0, 0);
}
