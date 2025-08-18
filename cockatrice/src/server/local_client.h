#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include "abstract_client.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(LocalClientLog, "local_client");

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
