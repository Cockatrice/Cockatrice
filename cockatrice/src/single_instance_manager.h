#ifndef COCKATRICE_SINGLE_INSTANCE_MANAGER_H
#define COCKATRICE_SINGLE_INSTANCE_MANAGER_H

#include <QDataStream>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>

class SingleInstanceManager : public QObject
{
    Q_OBJECT
public:
    explicit SingleInstanceManager(QObject *parent = nullptr) : QObject(parent)
    {
    }

    bool tryRun(const QStringList &initialFiles)
    {
        serverName = "CockatriceSingleInstance";

        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(100)) {
            // Another instance is running, send files/URLs to it
            QDataStream out(&socket);
            out << initialFiles;
            socket.flush();
            socket.waitForBytesWritten(1000);

            return false; // Do not continue in this process
        }

        // No other instance, create server
        server = new QLocalServer(this);
        connect(server, &QLocalServer::newConnection, this, &SingleInstanceManager::receiveFiles);
        if (!server->listen(serverName)) {
            QLocalServer::removeServer(serverName);
            server->listen(serverName);
        }
        return true;
    }

signals:
    void filesReceived(const QStringList &files);

private slots:
    void receiveFiles()
    {
        QLocalSocket *clientConnection = server->nextPendingConnection();
        connect(clientConnection, &QLocalSocket::disconnected, clientConnection, &QLocalSocket::deleteLater);
        clientConnection->waitForReadyRead(1000);

        QDataStream in(clientConnection);
        QStringList files;
        in >> files;

        emit filesReceived(files);
        clientConnection->disconnectFromServer();
    }

private:
    QString serverName;
    QLocalServer *server = nullptr;
};

#endif // COCKATRICE_SINGLE_INSTANCE_MANAGER_H
