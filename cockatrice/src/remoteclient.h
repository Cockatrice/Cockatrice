#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QTcpSocket>
#include "abstractclient.h"

class QTimer;

class RemoteClient : public AbstractClient {
    Q_OBJECT
signals:
    void maxPingTime(int seconds, int maxSeconds);
    void serverTimeout();
    void loginError(Response::ResponseCode resp, QString reasonStr, quint32 endTime);
    void socketError(const QString &errorString);
    void protocolVersionMismatch(int clientVersion, int serverVersion);
    void protocolError();
    void sigConnectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void sigDisconnectFromServer();
private slots:
    void slotConnected();
    void readData();
    void slotSocketError(QAbstractSocket::SocketError error);
    void ping();
    void processServerIdentificationEvent(const Event_ServerIdentification &event);
    void processConnectionClosedEvent(const Event_ConnectionClosed &event);
    void loginResponse(const Response &response);
    void doConnectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void doDisconnectFromServer();
private:
    static const int maxTimeout = 10;
    int timeRunning, lastDataReceived;

    QByteArray inputBuffer;
    bool messageInProgress;
    bool handshakeStarted;
    int messageLength;
    
    QTimer *timer;
    QTcpSocket *socket;
protected slots:    
    void sendCommandContainer(const CommandContainer &cont);
public:
    RemoteClient(QObject *parent = 0);
    ~RemoteClient();
    QString peerName() const { return socket->peerName(); }
    void connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void disconnectFromServer();
};

#endif
