#ifndef NETWORKSERVERTHREAD_H
#define NETWORKSERVERTHREAD_H

#include <QThread>
#include <QSslCertificate>
#include <QSslKey>
#include <QWaitCondition>

class Servatrice;
class NetworkServerInterface;
class QSslSocket;

class NetworkServerThread : public QThread {
	Q_OBJECT
private:
	Servatrice *server;
	NetworkServerInterface *interface;
	QSslCertificate cert;
	QSslKey privateKey;
	QString peerHostName, peerAddress;
	int peerPort;
	QSslCertificate peerCert;
	int socketDescriptor;
	QSslSocket *socket;
	enum ConnectionType { ClientType, ServerType } connectionType;
	
	void initClient();
	void initServer();
public:
	NetworkServerThread(int _socketDescriptor, Servatrice *_server, const QSslCertificate &_cert, const QSslKey &_privateKey, QObject *parent = 0);
	NetworkServerThread(const QString &_hostName, const QString &_address, int _port, const QSslCertificate &peerCert, Servatrice *_server, const QSslCertificate &_cert, const QSslKey &_privateKey, QObject *parent = 0);
	~NetworkServerThread();
	QWaitCondition initWaitCondition;
protected:
	void run();
};

#endif
