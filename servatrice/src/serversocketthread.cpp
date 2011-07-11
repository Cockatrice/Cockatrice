#include "serversocketthread.h"
#include "serversocketinterface.h"
#include "server_logger.h"
#include "main.h"

ServerSocketThread::ServerSocketThread(int _socketDescriptor, Servatrice *_server, QObject *parent)
	: QThread(parent), server(_server), socketDescriptor(_socketDescriptor)
{
}

ServerSocketThread::~ServerSocketThread()
{
	quit();
	wait();
}

void ServerSocketThread::run()
{
	QTcpSocket *socket = new QTcpSocket;
	socket->setSocketDescriptor(socketDescriptor);
	logger->logMessage(QString("incoming connection: %1").arg(socket->peerAddress().toString()));
	
	ssi = new ServerSocketInterface(server, socket);
	connect(ssi, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	
	exec();
}
