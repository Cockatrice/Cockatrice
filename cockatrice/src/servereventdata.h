#ifndef SERVEREVENTDATA_H
#define SERVEREVENTDATA_H

#include <QStringList>
#include <QHash>

enum ServerEventType {
	eventInvalid,
	eventPlayerId,
	eventSay,
	eventName,
	eventJoin,
	eventLeave,
	eventReadyStart,
	eventSetupZones,
	eventGameStart,
	eventShuffle,
	eventRollDice,
	eventDraw,
	eventMoveCard,
	eventCreateToken,
	eventSetCardAttr,
	eventAddCounter,
	eventSetCounter,
	eventDelCounter,
	eventSetActivePlayer,
	eventSetActivePhase,
	eventDumpZone,
	eventStopDumpZone
};

class ServerEventData {
private:
	static QHash<QString, ServerEventType> eventHash;
	
	bool IsPublic;
	int PlayerId;
	QString PlayerName;
	ServerEventType EventType;
	QStringList EventData;
public:
	ServerEventData(const QString &line);
	bool getPublic() const { return IsPublic; }
	int getPlayerId() const { return PlayerId; }
	const QString &getPlayerName() const { return PlayerName; }
	ServerEventType getEventType() const { return EventType; }
	const QStringList &getEventData() const { return EventData; }
};

enum ChatEventType {
	eventChatInvalid,
	eventChatListChannels,
	eventChatJoinChannel,
	eventChatListPlayers,
	eventChatLeaveChannel,
	eventChatSay
};

class ChatEventData {
private:
	static QHash<QString, ChatEventType> eventHash;
	
	ChatEventType eventType;
	QStringList eventData;
public:
	ChatEventData(const QString &line);
	ChatEventType getEventType() const { return eventType; }
	const QStringList &getEventData() const { return eventData; }
};

#endif
