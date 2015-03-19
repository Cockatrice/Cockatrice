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
class Command_JoinGame;

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
	void joinGameCommandReceived(const Command_JoinGame &cmd, int cmdId, int roomId, int serverId, qint64 sessionId);
	void gameCommandContainerReceived(const CommandContainer &cont, int playerId, int serverId, qint64 sessionId);
	void responseReceived(const Response &resp, qint64 sessionId);
	void gameEventContainerReceived(const GameEventContainer &cont, qint64 sessionId);
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
	void sessionEvent_UserJoined(const Event_UserJoined &event);
	void sessionEvent_UserLeft(const Event_UserLeft &event);
	
	void roomEvent_UserJoined(int roomId, const Event_JoinRoom &event);
	void roomEvent_UserLeft(int roomId, const Event_LeaveRoom &event);
	void roomEvent_Say(int roomId, const Event_RoomSay &event);
	void roomEvent_ListGames(int roomId, const Event_ListGames &event);
	
	void roomCommand_JoinGame(const Command_JoinGame &cmd, int cmdId, int roomId, qint64 sessionId);
	
	void processSessionEvent(const SessionEvent &event, qint64 sessionId);
	void processRoomEvent(const RoomEvent &event);
	void processRoomCommand(const CommandContainer &cont, qint64 sessionId);
	
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
