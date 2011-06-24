#ifndef LOCALSERVERINTERFACE_H
#define LOCALSERVERINTERFACE_H

#include "server_protocolhandler.h"

class LocalServer;

class LocalServerInterface : public Server_ProtocolHandler
{
	Q_OBJECT
private:
	DeckList *getDeckFromDatabase(int /*deckId*/) { return 0; }
	ResponseCode cmdAddToList(Command_AddToList * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdRemoveFromList(Command_RemoveFromList * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckList(Command_DeckList * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckNewDir(Command_DeckNewDir * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDelDir(Command_DeckDelDir * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDel(Command_DeckDel * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckUpload(Command_DeckUpload * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDownload(Command_DeckDownload * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdBanFromServer(Command_BanFromServer * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdShutdownServer(Command_ShutdownServer * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdUpdateServerMessage(Command_UpdateServerMessage * /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
public:
	LocalServerInterface(LocalServer *_server);
	~LocalServerInterface();
	
	void sendProtocolItem(ProtocolItem *item, bool deleteItem = true);
	
signals:
	void itemToClient(ProtocolItem *item);
public slots:
	void itemFromClient(ProtocolItem *item);
};

#endif