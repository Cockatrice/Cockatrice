#ifndef COCKATRICE_INTENT_CONNECT_TO_SERVER_H
#define COCKATRICE_INTENT_CONNECT_TO_SERVER_H

#include "contexts/context_connect_to_server.h"
#include "intent.h"
#include "intent_disconnect_from_server.h"
#include "remote_client.h"

#include <QTimer>

class IntentConnectToServer : public Intent
{
    Q_OBJECT

public:
    IntentConnectToServer(RemoteClient *_remoteClient, ContextConnectToServer *_context)
        : Intent(), remoteClient(_remoteClient), context(_context)
    {
    }

protected:
    bool checkPrecondition() const override
    {
        return remoteClient->getStatus() == ClientStatus::StatusDisconnected;
    }

    void onPreconditionSatisfied() override
    {
        remoteClient->connectToServer(context->hostname, context->port.toUInt(), context->username, context->password);
        connect(remoteClient, &RemoteClient::statusChanged, this, &IntentConnectToServer::onStatusChanged);
    }

    void onPreconditionNotSatisfied() override
    {
        runDependency(new IntentDisconnectFromServer(remoteClient));
    }

private:
    RemoteClient *remoteClient;
    ContextConnectToServer *context;
private slots:
    void onStatusChanged(ClientStatus status)
    {
        if (status == ClientStatus::StatusLoggedIn) {
            auto timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(2000);
            connect(timer, &QTimer::timeout, this, &IntentConnectToServer::finished);
            timer->start();
        }
    }
};

#endif // COCKATRICE_INTENT_CONNECT_TO_SERVER_H
