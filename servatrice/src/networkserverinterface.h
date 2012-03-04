#ifndef NETWORKSERVERINTERFACE_H
#define NETWORKSERVERINTERFACE_H

#include "servatrice.h"

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
	QMutex outputBufferMutex;
	Servatrice *server;
	QSslSocket *socket;
	
	QByteArray inputBuffer, outputBuffer;
	bool messageInProgress;
	int messageLength;
	
	void processMessage(const ServerNetworkMessage &item);
	void sharedCtor();
public:
	NetworkServerInterface(Servatrice *_server, QSslSocket *_socket);
	NetworkServerInterface(const QString &peerHostName, const QString &peerAddress, int peerPort, Servatrice *_server, QSslSocket *_socket);
	~NetworkServerInterface();
	
	void transmitMessage(const ServerNetworkMessage &item);
};

#endif
