#ifndef COCKATRICE_INTENT_DISCONNECT_FROM_SERVER_H
#define COCKATRICE_INTENT_DISCONNECT_FROM_SERVER_H
#include "intent.h"
#include "remote_client.h"

class IntentDisconnectFromServer : public Intent
{
    Q_OBJECT

public:
    IntentDisconnectFromServer(RemoteClient *_remoteClient) : Intent(), remoteClient(_remoteClient)
    {
    }

protected:
    bool checkPrecondition() const override
    {
        return remoteClient->getStatus() == ClientStatus::StatusDisconnected;
    }

    void onPreconditionSatisfied() override
    {
        qWarning() << "Client disconnected, disconnect is finished";
        emit finished();
    }

    void onPreconditionNotSatisfied() override
    {
        qWarning() << "Client not disconnected, hooking up signal and disconnecting." << remoteClient->getStatus();
        connect(remoteClient, &RemoteClient::statusChanged, this, &IntentDisconnectFromServer::onStatusChanged);
        remoteClient->disconnectFromServer();
    }

private:
    RemoteClient *remoteClient;
private slots:
    void onStatusChanged(ClientStatus status)
    {
        qWarning() << "Client Status changed: " << status;
        if (status == ClientStatus::StatusDisconnected) {
            qWarning() << "Client disconnected, finished";
            emit finished();
        }
    }
};

#endif // COCKATRICE_INTENT_DISCONNECT_FROM_SERVER_H
