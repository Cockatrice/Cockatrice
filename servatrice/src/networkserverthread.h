#ifndef NETWORKSERVERTHREAD_H
#define NETWORKSERVERTHREAD_H

#include <QThread>
#include <QSslCertificate>
#include <QSslKey>

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
	int socketDescriptor;
	QSslSocket *socket;
public:
	NetworkServerThread(int _socketDescriptor, Servatrice *_server, const QSslCertificate &_cert, const QSslKey &_privateKey, QObject *parent = 0);
	~NetworkServerThread();
protected:
	void run();
};

#endif
