#ifndef ISL_INTERFACE_H
#define ISL_INTERFACE_H

#include "servatrice.h"
#include <QSslCertificate>
#include <QWaitCondition>
#include "pb/serverinfo_user.pb.h"

class Servatrice;
class QSslSocket;
class QSslKey;
class IslMessage;

class Event_ServerCompleteList;
class Event_UserMessage;
class Event_UserJoined;
class Event_UserLeft;

class IslInterface : public QObject {
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
	void flushOutputBuffer();
signals:
	void outputBufferChanged();
	
	void externalUserJoined(ServerInfo_User userInfo);
	void externalUserLeft(QString name);
private:
	int serverId;
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
	
	void sessionEvent_ServerCompleteList(const Event_ServerCompleteList &event);
	void sessionEvent_UserMessage(const SessionEvent &sessionEvent, const Event_UserMessage &event);
	void sessionEvent_UserJoined(const Event_UserJoined &event);
	void sessionEvent_UserLeft(const Event_UserLeft &event);
	
	void processSessionEvent(const SessionEvent &event);
	
	void processMessage(const IslMessage &item);
	void sharedCtor(const QSslCertificate &cert, const QSslKey &privateKey);
public slots:
	void initServer();
	void initClient();
public:
	IslInterface(int socketDescriptor, const QSslCertificate &cert, const QSslKey &privateKey, Servatrice *_server);
	IslInterface(int _serverId, const QString &peerHostName, const QString &peerAddress, int peerPort, const QSslCertificate &peerCert, const QSslCertificate &cert, const QSslKey &privateKey, Servatrice *_server);
	~IslInterface();
	
	void transmitMessage(const IslMessage &item);
};

#endif
