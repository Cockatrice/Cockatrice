#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include "../client/game_logic/abstract_client.h"

class LocalServerInterface;

class LocalClient : public AbstractClient
{
    Q_OBJECT
private:
    LocalServerInterface *lsi;

public:
    LocalClient(LocalServerInterface *_lsi,
                const QString &_playerName,
                const QString &_clientId,
                QObject *parent = nullptr);
    ~LocalClient() override;

    void sendCommandContainer(const CommandContainer &cont) override;
private slots:
    void itemFromServer(const ServerMessage &item);
};

#endif
