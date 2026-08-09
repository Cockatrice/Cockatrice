#include "intent_connect_to_server.h"

#include "intent_disconnect_from_server.h"

#include <QTimer>

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
    connect(remoteClient, &RemoteClient::socketError, this, &IntentConnectToServer::onSocketError);
    connect(
        remoteClient, &RemoteClient::loginError, this,
        [this](Response::ResponseCode, const QString &reason, quint32, const QList<QString> &) { emitFailed(reason); });

    QTimer::singleShot(15000, this, [this]() {
        emitFailed(tr("Timed out while connecting to %1:%2").arg(context->hostname, context->port));
    });
}

void IntentConnectToServer::onPreconditionNotSatisfied()
{
    runDependency(new IntentDisconnectFromServer(remoteClient));
}

void IntentConnectToServer::onStatusChanged(ClientStatus status)
{
    if (status == ClientStatus::StatusLoggedIn) {
        emitFinished();
    }
}

void IntentConnectToServer::onSocketError(const QString &errorString)
{
    emitFailed(tr("Failed to connect to %1:%2: %3").arg(context->hostname, context->port, errorString));
}
