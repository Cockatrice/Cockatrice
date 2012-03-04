#ifndef LOCALSERVERINTERFACE_H
#define LOCALSERVERINTERFACE_H

#include "server_protocolhandler.h"

class LocalServer;

class LocalServerInterface : public Server_ProtocolHandler
{
	Q_OBJECT
private:
	DeckList *getDeckFromDatabase(int /*deckId*/) { return 0; }
	Response::ResponseCode cmdAddToList(const Command_AddToList & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdRemoveFromList(const Command_RemoveFromList & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdDeckList(const Command_DeckList & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdDeckNewDir(const Command_DeckNewDir & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdDeckDelDir(const Command_DeckDelDir & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdDeckDel(const Command_DeckDel & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdDeckUpload(const Command_DeckUpload & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdDeckDownload(const Command_DeckDownload & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdReplayList(const Command_ReplayList & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdReplayDownload(const Command_ReplayDownload & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdReplayModifyMatch(const Command_ReplayModifyMatch &cmd, ResponseContainer &rc) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdBanFromServer(const Command_BanFromServer & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdShutdownServer(const Command_ShutdownServer & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
	Response::ResponseCode cmdUpdateServerMessage(const Command_UpdateServerMessage & /*cmd*/, ResponseContainer & /*rc*/) { return Response::RespFunctionNotAllowed; }
public:
	LocalServerInterface(LocalServer *_server);
	~LocalServerInterface();
	
	QString getAddress() const { return QString(); }
	void transmitProtocolItem(const ServerMessage &item);
signals:
	void itemToClient(const ServerMessage &item);
public slots:
	void itemFromClient(const CommandContainer &item);
};

#endif