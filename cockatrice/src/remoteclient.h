#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QTcpSocket>
#include "abstractclient.h"

class QTimer;
class QXmlStreamReader;
class QXmlStreamWriter;

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
	void loginResponse(ProtocolResponse *response);
private:
	static const int maxTimeout = 10;
	
	QTimer *timer;
	QTcpSocket *socket;
	QXmlStreamReader *xmlReader;
	QXmlStreamWriter *xmlWriter;
	TopLevelProtocolItem *topLevelItem;
public:
	RemoteClient(QObject *parent = 0);
	~RemoteClient();
	QString peerName() const { return socket->peerName(); }

	void connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
	void disconnectFromServer();
	
	void sendCommandContainer(CommandContainer *cont);
};

#endif
