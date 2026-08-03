#include "intent_connect_to_server.h"

#include "intent_disconnect_from_server.h"

IntentConnectToServer::IntentConnectToServer(RemoteClient *_remoteClient, ContextConnectToServer *_context)
    : Intent(), remoteClient(_remoteClient), context(_context)
{
}

bool IntentConnectToServer::checkPrecondition() const
{
    return remoteClient->getStatus() == ClientStatus::StatusDisconnected;
}

void IntentConnectToServer::onPreconditionSatisfied()
{
    remoteClient->connectToServer(context->hostname, context->port.toUInt(), context->username, context->password);
    connect(remoteClient, &RemoteClient::statusChanged, this, &IntentConnectToServer::onStatusChanged);
    connect(remoteClient, &RemoteClient::loginError, this,
            [this](Response::ResponseCode, const QString &reason, quint32, const QList<QString> &) {
                emit failed(reason);
            });
}

void IntentConnectToServer::onPreconditionNotSatisfied()
{
    runDependency(new IntentDisconnectFromServer(remoteClient));
}

void IntentConnectToServer::onStatusChanged(ClientStatus status)
{
    if (status == ClientStatus::StatusLoggedIn) {
        emit finished();
    }
}
