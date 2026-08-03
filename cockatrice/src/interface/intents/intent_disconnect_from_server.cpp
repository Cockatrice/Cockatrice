#include "intent_disconnect_from_server.h"

IntentDisconnectFromServer::IntentDisconnectFromServer(RemoteClient *_remoteClient)
    : Intent(), remoteClient(_remoteClient)
{
}

bool IntentDisconnectFromServer::checkPrecondition() const
{
    return remoteClient->getStatus() == ClientStatus::StatusDisconnected;
}

void IntentDisconnectFromServer::onPreconditionSatisfied()
{
    emit finished();
}

void IntentDisconnectFromServer::onPreconditionNotSatisfied()
{
    connect(remoteClient, &RemoteClient::statusChanged, this, &IntentDisconnectFromServer::onStatusChanged);
    remoteClient->disconnectFromServer();
}

void IntentDisconnectFromServer::onStatusChanged(ClientStatus status)
{
    if (status == ClientStatus::StatusDisconnected) {
        emit finished();
    }
}
