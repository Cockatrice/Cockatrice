#ifndef SERVEREVENTDATA_H
#define SERVEREVENTDATA_H

#include <QStringList>


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
	eventDumpZone
};

struct event_string {
	ServerEventType type;
	char *str;
};

extern const int event_count;
extern const event_string event_strings[];

class ServerEventData {
private:
	bool IsPublic;
	int PlayerId;
	QString PlayerName;
	ServerEventType EventType;
	QStringList EventData;
public:
	ServerEventData(const QString &line);
	bool getPublic() { return IsPublic; }
	int getPlayerId() { return PlayerId; }
	QString getPlayerName() { return PlayerName; }
	ServerEventType getEventType() { return EventType; }
	QStringList getEventData() { return EventData; }
};

#endif
