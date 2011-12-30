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
	void socketError(const QString &errorString);
	void protocolVersionMismatch(int clientVersion, int serverVersion);
	void protocolError();
private slots:
	void slotConnected();
	void readData();
	void slotSocketError(QAbstractSocket::SocketError error);
	void ping();
	void processServerIdentificationEvent(const Event_ServerIdentification &event);
	void loginResponse(const Response &response);
private:
	static const int maxTimeout = 10;
	int timeRunning, lastDataReceived;

	QByteArray inputBuffer;
	bool messageInProgress;
	int messageLength;
	
	QTimer *timer;
	QTcpSocket *socket;
	
	void sendCommandContainer(const CommandContainer &cont);
public:
	RemoteClient(QObject *parent = 0);
	~RemoteClient();
	QString peerName() const { return socket->peerName(); }

	void connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
	void disconnectFromServer();
};

#endif
