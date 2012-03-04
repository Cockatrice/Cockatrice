#include "networkserverthread.h"
#include "networkserverinterface.h"
#include "server_logger.h"
#include "servatrice.h"
#include "main.h"
#include <QSslSocket>

NetworkServerThread::NetworkServerThread(int _socketDescriptor, Servatrice *_server, const QSslCertificate &_cert, const QSslKey &_privateKey, QObject *parent)
	: QThread(parent), server(_server), socketDescriptor(_socketDescriptor), cert(_cert), privateKey(_privateKey)
{
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

NetworkServerThread::~NetworkServerThread()
{
	quit();
	wait();
	
	delete socket;
}

void NetworkServerThread::run()
{
	socket = new QSslSocket;
	socket->setSocketDescriptor(socketDescriptor);
	socket->setLocalCertificate(cert);
	socket->setPrivateKey(privateKey);
	
	logger->logMessage(QString("[SN] incoming connection: %1").arg(socket->peerAddress().toString()));
	
	QList<ServerProperties> serverList = server->getServerList();
	int listIndex = -1;
	for (int i = 0; i < serverList.size(); ++i)
		if (serverList[i].address == socket->peerAddress()) {
			listIndex = i;
			break;
		}
	if (listIndex == -1) {
		logger->logMessage(QString("[SN] address %1 unknown, terminating connection").arg(socket->peerAddress().toString()));
		return;
	}
	
	socket->startServerEncryption();
	if (!socket->waitForEncrypted(5000)) {
		logger->logMessage(QString("[SN] SSL handshake timeout, terminating connection"));
		return;
	}
	
	if (serverList[listIndex].cert == socket->peerCertificate())
		logger->logMessage(QString("[SN] Peer authenticated as " + serverList[listIndex].hostname));
	else {
		logger->logMessage(QString("[SN] Authentication failed, terminating connection"));
		return;
	}
	
	interface = new NetworkServerInterface(server, socket);
	connect(interface, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	
	exec();
}
