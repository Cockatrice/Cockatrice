#include "installer_close_server.h"

#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <libcockatrice/utility/installer_close_protocol.h>
#include <libcockatrice/utility/local_server_name.h>

namespace
{
/// The one line the probe writes, from the header both ends include. A QByteArray view of
/// the shared literal, so the client cannot drift from the probe's copy.
const QByteArray CLOSE_COMMAND = QByteArray::fromRawData(INSTALLER_CLOSE_COMMAND, sizeof(INSTALLER_CLOSE_COMMAND) - 1);

/// How long a connection that never sends the command may stay open. The probe writes and
/// closes at once, so this only reaps a stray client; it must not run while a close request
/// is being handled, which is why it is a timer on the socket and not a disconnected slot.
constexpr int STRAY_CONNECTION_MILLISECONDS = 5000;
} // namespace

InstallerCloseServer::InstallerCloseServer(QObject *parent) : QObject(parent)
{
}

QString InstallerCloseServer::socketNameForUser(qint64 processId, const QString &userName)
{
    return QStringLiteral("CockatriceInstallerClose-%1-%2").arg(processId).arg(userName);
}

QString InstallerCloseServer::socketName(qint64 processId)
{
    // The same scoping helper the single-instance socket uses, so the probe only has to
    // reproduce one rule (USER, then USERNAME, then the home directory name).
    return scopedLocalServerName(QStringLiteral("CockatriceInstallerClose-%1").arg(processId));
}

bool InstallerCloseServer::listen()
{
    if (server != nullptr) {
        return server->isListening();
    }
    name = socketName(QCoreApplication::applicationPid());
    server = new QLocalServer(this);
    // A crashed instance can leave a Unix socket file behind; on Windows this is a no-op.
    QLocalServer::removeServer(name);
    if (!server->listen(name)) {
        delete server;
        server = nullptr;
        return false;
    }

    connect(server, &QLocalServer::newConnection, this, [this]() {
        while (QLocalSocket *socket = server->nextPendingConnection()) {
            // No disconnected -> deleteLater() slot: the close request can spin a nested
            // event loop, the probe disconnects while it runs, and a deferred delete would
            // then free the socket from inside its own readyRead emission. The reaper covers
            // a client that never sends the line; the handler below deletes after the emit.
            auto *reaper = new QTimer(socket);
            reaper->setSingleShot(true);
            connect(reaper, &QTimer::timeout, socket, &QObject::deleteLater);
            reaper->start(STRAY_CONNECTION_MILLISECONDS);
            connect(socket, &QLocalSocket::readyRead, this, [this, socket, reaper]() {
                if (!socket->canReadLine()) {
                    return;
                }
                if (socket->readLine() != CLOSE_COMMAND) {
                    return;
                }
                // Stop the reaper before the emit: the receiver may close the window and run
                // a nested event loop for as long as the user takes, and the reaper must not
                // free the socket from inside that loop while this handler still holds it.
                reaper->stop();
                emit closeRequested();
                socket->disconnectFromServer();
                socket->deleteLater();
            });
        }
    });
    return true;
}
