/**
 * @file local_server_interface.h
 * @ingroup Server
 * @brief TODO: Document this.
 */

#ifndef LOCALSERVERINTERFACE_H
#define LOCALSERVERINTERFACE_H

#include <../remote/server_protocolhandler.h>

class LocalServer;

class LocalServerInterface : public Server_ProtocolHandler
{
    Q_OBJECT
public:
    LocalServerInterface(LocalServer *_server, Server_DatabaseInterface *_databaseInterface);
    ~LocalServerInterface() override;

    QString getAddress() const override
    {
        return QString();
    }
    QString getConnectionType() const override
    {
        return "local";
    }
    void transmitProtocolItem(const ServerMessage &item) override;
signals:
    void itemToClient(const ServerMessage &item);
public slots:
    void itemFromClient(const CommandContainer &item);
};

#endif
