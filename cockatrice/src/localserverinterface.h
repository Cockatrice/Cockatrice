#ifndef LOCALSERVERINTERFACE_H
#define LOCALSERVERINTERFACE_H

#include "server_protocolhandler.h"

class LocalServer;

class LocalServerInterface : public Server_ProtocolHandler
{
    Q_OBJECT
public:
    LocalServerInterface(LocalServer *_server, Server_DatabaseInterface *_databaseInterface);
    ~LocalServerInterface();
    
    QString getAddress() const { return QString(); }
    QString getConnectionType() const { return "local"; };
    void transmitProtocolItem(const ServerMessage &item);
signals:
    void itemToClient(const ServerMessage &item);
public slots:
    void itemFromClient(const CommandContainer &item);
};

#endif
