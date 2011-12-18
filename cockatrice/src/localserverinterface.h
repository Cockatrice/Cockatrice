#ifndef LOCALSERVERINTERFACE_H
#define LOCALSERVERINTERFACE_H

#include "server_protocolhandler.h"

class LocalServer;

class LocalServerInterface : public Server_ProtocolHandler
{
	Q_OBJECT
private:
	DeckList *getDeckFromDatabase(int /*deckId*/) { return 0; }
	ResponseCode cmdAddToList(const Command_AddToList & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdRemoveFromList(const Command_RemoveFromList & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckList(const Command_DeckList & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckNewDir(const Command_DeckNewDir & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDelDir(const Command_DeckDelDir & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDel(const Command_DeckDel & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckUpload(const Command_DeckUpload & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdDeckDownload(const Command_DeckDownload & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdBanFromServer(const Command_BanFromServer & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdShutdownServer(const Command_ShutdownServer & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
	ResponseCode cmdUpdateServerMessage(const Command_UpdateServerMessage & /*cmd*/, BlaContainer * /*bla*/) { return RespFunctionNotAllowed; }
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
	void itemFromClient(const CommandContainer &item);
};

#endif