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

    // Hand off to an already-running primary instance if one exists.
    if (forwardToPrimary(filesToSend)) {
        return false;
    }

    // No primary instance is currently reachable, so become the primary.
    server = new QLocalServer(this);
    connect(server, &QLocalServer::newConnection, this, &SingleInstanceManager::handleNewConnection);

    if (server->listen(serverName)) {
        return true;
    }

    // Another instance may have started while we were probing; hand off to it
    // instead of stealing its socket.
    if (forwardToPrimary(filesToSend)) {
        return false;
    }

    // The socket is stale (left over by a crashed instance): remove it and
    // retry. If that still fails, another instance just took the name.
    QLocalServer::removeServer(serverName);
    if (server->listen(serverName)) {
        return true;
    }

    forwardToPrimary(filesToSend);
    return false;
}

bool SingleInstanceManager::forwardToPrimary(const QStringList &filesToSend)
{
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (!socket.waitForConnected(200)) {
        return false;
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

    // Only report a successful hand-off once the primary has acknowledged that
    // it actually read the payload. A socket that connects but never answers
    // belongs to a process that is dying, so the caller must not treat this as
    // a hand-off (otherwise it would exit without anyone handling the files).
    if (!socket.waitForReadyRead(1000)) {
        return false;
    }
    return socket.readAll() == ACK_MESSAGE;
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

            emit filesReceived(files);

            // Reset buffer (single message use-case)
            buffer->clear();
            *expectedSize = 0;

            socket->disconnectFromServer();
            return;
        }
    });

    connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}
