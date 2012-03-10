#ifndef NETWORKSERVERINTERFACE_H
#define NETWORKSERVERINTERFACE_H

#include "servatrice.h"
#include <QSslCertificate>
#include <QSslKey>
#include <QWaitCondition>

class Servatrice;
class QSslSocket;
class ServerNetworkMessage;

class NetworkServerInterface : public QObject {
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
	void flushOutputBuffer();
signals:
	void outputBufferChanged();
private:
	int socketDescriptor;
	QString peerHostName, peerAddress;
	int peerPort;
	QSslCertificate peerCert;
	
	QMutex outputBufferMutex;
	Servatrice *server;
	QSslSocket *socket;
	
	QByteArray inputBuffer, outputBuffer;
	bool messageInProgress;
	int messageLength;
	
	void processMessage(const ServerNetworkMessage &item);
	void sharedCtor(const QSslCertificate &cert, const QSslKey &privateKey);
public slots:
	void initServer();
	void initClient();
public:
	NetworkServerInterface(int socketDescriptor, const QSslCertificate &cert, const QSslKey &privateKey, Servatrice *_server);
	NetworkServerInterface(const QString &peerHostName, const QString &peerAddress, int peerPort, const QSslCertificate &peerCert, const QSslCertificate &cert, const QSslKey &privateKey, Servatrice *_server);
	~NetworkServerInterface();
	
	void transmitMessage(const ServerNetworkMessage &item);
};

#endif
