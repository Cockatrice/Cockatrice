#include "single_instance_manager.h"

#include <QDir>

namespace
{
// Sent by the primary instance after it has read a forwarded payload. Without
// an acknowledgment, a second instance cannot tell a live primary apart from a
// stale socket left behind by a process that is still shutting down.
const QByteArray ACK_MESSAGE = QByteArrayLiteral("COCKATRICE_ACK");
} // namespace

SingleInstanceManager::SingleInstanceManager(QObject *parent) : QObject(parent)
{
}

bool SingleInstanceManager::tryRun(const QStringList &filesToSend)
{
    // Scope the socket name to the current user. On Linux the default abstract
    // namespace is system-wide, so a plain name would let one user's instance
    // hijack another user's session.
    QString userName = qEnvironmentVariable("USER");
    if (userName.isEmpty()) {
        userName = qEnvironmentVariable("USERNAME");
    }
    if (userName.isEmpty()) {
        userName = QDir::home().dirName();
    }
    serverName = QStringLiteral("CockatriceSingleInstance-%1").arg(userName);

    // Hand off to an already-running primary instance if one exists. Never steal
    // the socket of a busy primary: it is alive and will act on the payload.
    switch (forwardToPrimary(filesToSend)) {
        case ForwardResult::Delivered:
            return false;
        case ForwardResult::PrimaryBusy:
            return false;
        case ForwardResult::NoPrimary:
            break;
    }

    // No primary instance is currently reachable, so become the primary.
    server = new QLocalServer(this);
    connect(server, &QLocalServer::newConnection, this, &SingleInstanceManager::handleNewConnection);

    if (server->listen(serverName)) {
        return true;
    }

    // Another instance may have started while we were probing; hand off to it
    // instead of stealing its socket.
    switch (forwardToPrimary(filesToSend)) {
        case ForwardResult::Delivered:
            return false;
        case ForwardResult::PrimaryBusy:
            return false;
        case ForwardResult::NoPrimary:
            break;
    }

    // The socket is stale (left over by a crashed instance), so no primary is
    // holding it: remove it and retry. If that still fails, another instance
    // just took the name.
    QLocalServer::removeServer(serverName);
    if (server->listen(serverName)) {
        return true;
    }

    forwardToPrimary(filesToSend);
    return false;
}

SingleInstanceManager::ForwardResult SingleInstanceManager::forwardToPrimary(const QStringList &filesToSend)
{
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (!socket.waitForConnected(200)) {
        return ForwardResult::NoPrimary;
    }

    // Serialize payload with length prefix
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out << filesToSend;

    QByteArray message;
    QDataStream msgStream(&message, QIODevice::WriteOnly);
    msgStream << quint32(payload.size());
    message.append(payload);

    socket.write(message);
    socket.flush();
    socket.waitForBytesWritten(1000);

    // A plain launch has nothing for the primary to act on, so there is nothing
    // to acknowledge. Waiting here would block the new instance for seconds if
    // the primary is busy in a modal dialog, so only the activation path (which
    // needs the ACK to avoid stealing a live primary's socket) waits below.
    if (filesToSend.isEmpty()) {
        return ForwardResult::Delivered;
    }

    // Only report a successful hand-off once the primary has acknowledged that
    // it actually read the payload. A socket that connects but is still working
    // on an earlier payload is alive but busy, not dead: give it more room
    // before giving up, so a slow handler does not make a live primary look
    // dead (which would lead to stealing its socket).
    if (!socket.waitForReadyRead(1000) && !socket.waitForReadyRead(4000)) {
        return ForwardResult::PrimaryBusy;
    }
    return socket.readAll() == ACK_MESSAGE ? ForwardResult::Delivered : ForwardResult::PrimaryBusy;
}

void SingleInstanceManager::handleNewConnection()
{
    QLocalSocket *socket = server->nextPendingConnection();

    // Per-connection state. QSharedPointer keeps the buffers alive for as long
    // as the connection handler is attached to the socket.
    auto buffer = QSharedPointer<QByteArray>::create();
    auto expectedSize = QSharedPointer<quint32>::create(0);

    connect(socket, &QLocalSocket::readyRead, this, [this, socket, buffer, expectedSize]() {
        buffer->append(socket->readAll());

        QDataStream stream(buffer.data(), QIODevice::ReadOnly);

        while (true) {
            // Step 1: read size
            if (*expectedSize == 0) {
                if (buffer->size() < static_cast<int>(sizeof(quint32))) {
                    return;
                }

                stream >> *expectedSize;
            }

            // Step 2: wait for full payload
            if (buffer->size() < static_cast<int>(sizeof(quint32) + *expectedSize)) {
                return;
            }

            // Step 3: extract payload
            QByteArray payload = buffer->mid(sizeof(quint32), *expectedSize);

            QDataStream payloadStream(&payload, QIODevice::ReadOnly);
            QStringList files;
            payloadStream >> files;

            // Acknowledge receipt as soon as the payload is parsed, before the
            // primary starts handling it. The handlers run synchronously and can
            // take longer than the sender's readiness timeout (e.g. a modal
            // confirmation box), which would otherwise make a live primary look
            // dead and cause duplicate handling.
            socket->write(ACK_MESSAGE);
            socket->flush();

            // Drop the payload from the buffer before handling it: the handlers
            // run synchronously and can spin a nested event loop (e.g. a modal
            // dialog) that re-reads this socket, which would re-parse and re-emit
            // the same files.
            buffer->clear();
            *expectedSize = 0;

            emit filesReceived(files);

            socket->disconnectFromServer();
            return;
        }
    });

    connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}
