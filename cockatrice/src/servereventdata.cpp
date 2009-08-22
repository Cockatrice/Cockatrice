#include "servereventdata.h"

// Message structure for server events:
// {"private","public"}|PlayerId|PlayerName|EventType|EventData

QHash<QString, ServerEventType> ServerEventData::eventHash;

ServerEventData::ServerEventData(const QString &line)
{
	if (eventHash.isEmpty()) {
		eventHash.insert("player_id", eventPlayerId);
		eventHash.insert("say", eventSay);
		eventHash.insert("name", eventName);
		eventHash.insert("join", eventJoin);
		eventHash.insert("leave", eventLeave);
		eventHash.insert("ready_start", eventReadyStart);
		eventHash.insert("setup_zones", eventSetupZones);
		eventHash.insert("game_start", eventGameStart);
		eventHash.insert("shuffle", eventShuffle);
		eventHash.insert("roll_dice", eventRollDice);
		eventHash.insert("draw", eventDraw);
		eventHash.insert("move_card", eventMoveCard);
		eventHash.insert("create_token", eventCreateToken);
		eventHash.insert("set_card_attr", eventSetCardAttr);
		eventHash.insert("add_counter", eventAddCounter);
		eventHash.insert("set_counter", eventSetCounter);
		eventHash.insert("del_counter", eventDelCounter);
		eventHash.insert("set_active_player", eventSetActivePlayer);
		eventHash.insert("set_active_phase", eventSetActivePhase);
		eventHash.insert("dump_zone", eventDumpZone);
		eventHash.insert("stop_dump_zone", eventStopDumpZone);
	}
	
	QStringList values = line.split('|');

	IsPublic = !values.takeFirst().compare("public");
	PlayerId = values.takeFirst().toInt();
	PlayerName = values.takeFirst();
	EventType = eventHash.value(values.takeFirst(), eventInvalid);
	EventData = values;
}

QHash<QString, ChatEventType> ChatEventData::eventHash;

ChatEventData::ChatEventData(const QString &line)
{
	if (eventHash.isEmpty()) {
		eventHash.insert("list_channels", eventChatListChannels);
		eventHash.insert("join_channel", eventChatJoinChannel);
		eventHash.insert("list_players", eventChatListPlayers);
		eventHash.insert("leave_channel", eventChatLeaveChannel);
		eventHash.insert("say", eventChatSay);
	}
	
	QStringList values = line.split('|');
	values.removeFirst();
	eventType = eventHash.value(values.takeFirst(), eventChatInvalid);
	eventData = values;
}
