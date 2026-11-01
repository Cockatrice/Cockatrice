/**
 * @file local_client.h
 * @ingroup Client
 */
//! \todo Document this file.

#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include "../abstract/abstract_client.h"

#include <QLoggingCategory>
#include <QPointer>
#include <QString>
#include <qtmetamacros.h>

class QObject;
class ServerMessage;

inline Q_LOGGING_CATEGORY(LocalClientLog, "local_client");

class LocalServerInterface;

class LocalClient : public AbstractClient
{
    Q_OBJECT
private:
    // Guarded pointer: the LocalServer (and its interfaces) may be destroyed while this client
    // still has pending commands queued on the event loop.
    QPointer<LocalServerInterface> lsi;

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
