#include "single_instance_manager.h"

SingleInstanceManager::SingleInstanceManager(QObject *parent) : QObject(parent)
{
}

bool SingleInstanceManager::tryRun(const QStringList &filesToSend)
{
    serverName = "CockatriceSingleInstance";

    // Attempt to connect only if we have files to send
    if (!filesToSend.isEmpty()) {
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(200)) {
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

            return false; // Sent successfully → exit
        }
    }

    // Otherwise, start server
    server = new QLocalServer(this);
    connect(server, &QLocalServer::newConnection, this, &SingleInstanceManager::handleNewConnection);

    if (!server->listen(serverName)) {
        QLocalServer::removeServer(serverName);
        server->listen(serverName);
    }

    return true; // This process is now primary server
}

void SingleInstanceManager::handleNewConnection()
{
    QLocalSocket *socket = server->nextPendingConnection();

    // Per-connection state
    auto buffer = new QByteArray();
    auto expectedSize = new quint32(0);

    connect(socket, &QLocalSocket::readyRead, this, [this, socket, buffer, expectedSize]() {
        buffer->append(socket->readAll());

        QDataStream stream(buffer, QIODevice::ReadOnly);

        while (true) {
            // Step 1: read size
            if (*expectedSize == 0) {
                if (buffer->size() < static_cast<int>(sizeof(quint32)))
                    return;

                stream >> *expectedSize;
            }

            // Step 2: wait for full payload
            if (buffer->size() < static_cast<int>(sizeof(quint32) + *expectedSize))
                return;

            // Step 3: extract payload
            QByteArray payload = buffer->mid(sizeof(quint32), *expectedSize);

            QDataStream payloadStream(&payload, QIODevice::ReadOnly);
            QStringList files;
            payloadStream >> files;

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