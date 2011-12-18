#ifndef LOCALSERVERINTERFACE_H
#define LOCALSERVERINTERFACE_H

#include "server_protocolhandler.h"

class LocalServer;

class LocalServerInterface : public Server_ProtocolHandler
{
	Q_OBJECT
private:
	DeckList *getDeckFromDatabase(int /*deckId*/) { return 0; }
	ResponseCode cmdAddToList(const Command_AddToList & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdRemoveFromList(const Command_RemoveFromList & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckList(const Command_DeckList & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckNewDir(const Command_DeckNewDir & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDelDir(const Command_DeckDelDir & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDel(const Command_DeckDel & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckUpload(const Command_DeckUpload & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDownload(const Command_DeckDownload & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdBanFromServer(const Command_BanFromServer & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdShutdownServer(const Command_ShutdownServer & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdUpdateServerMessage(const Command_UpdateServerMessage & /*cmd*/, CommandContainer * /*cont*/) { return RespFunctionNotAllowed; }
protected:
	bool getCompressionSupport() const { return false; }
public:
	LocalServerInterface(LocalServer *_server);
	~LocalServerInterface();
	
	QString getAddress() const { return QString(); }
	void sendProtocolItem(ProtocolItem *item, bool deleteItem = true);
signals:
	void itemToClient(ProtocolItem *item);
public slots:
	void itemFromClient(ProtocolItem *item);
};

#endif