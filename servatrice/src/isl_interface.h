#ifndef ISL_INTERFACE_H
#define ISL_INTERFACE_H

#include "servatrice.h"
#include <QSslCertificate>
#include <QWaitCondition>
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_room.pb.h"
#include "pb/serverinfo_game.pb.h"

class Servatrice;
class QSslSocket;
class QSslKey;
class IslMessage;

class Event_ServerCompleteList;
class Event_UserMessage;
class Event_UserJoined;
class Event_UserLeft;
class Event_JoinRoom;
class Event_LeaveRoom;
class Event_RoomSay;
class Event_ListGames;

class IslInterface : public QObject {
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
	void flushOutputBuffer();
signals:
	void outputBufferChanged();
	
	void externalUserJoined(ServerInfo_User userInfo);
	void externalUserLeft(QString userName);
	void externalRoomUserJoined(int roomId, ServerInfo_User userInfo);
	void externalRoomUserLeft(int roomId, QString userName);
	void externalRoomSay(int roomId, QString userName, QString message);
	void externalRoomGameListChanged(int roomId, ServerInfo_Game gameInfo);
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
	
	void roomEvent_UserJoined(int roomId, const Event_JoinRoom &event);
	void roomEvent_UserLeft(int roomId, const Event_LeaveRoom &event);
	void roomEvent_Say(int roomId, const Event_RoomSay &event);
	void roomEvent_ListGames(int roomId, const Event_ListGames &event);
	
	void processSessionEvent(const SessionEvent &event);
	void processRoomEvent(const RoomEvent &event);
	
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
