#include "servereventdata.h"

// Message structure for server events:
// {"private","public"}|PlayerId|PlayerName|EventType|EventData

const int event_count = 19;
const event_string event_strings[event_count] = {
	{eventPlayerId, "player_id"},
	{eventSay, "say"},
	{eventName, "name"},
	{eventJoin, "join"},
	{eventLeave, "leave"},
	{eventReadyStart, "ready_start"},
	{eventSetupZones, "setup_zones"},
	{eventGameStart, "game_start"},
	{eventShuffle, "shuffle"},
	{eventRollDice, "roll_dice"},
	{eventDraw, "draw"},
	{eventMoveCard, "move_card"},
	{eventCreateToken, "create_token"},
	{eventSetCardAttr, "set_card_attr"},
	{eventSetCounter, "set_counter"},
	{eventDelCounter, "del_counter"},
	{eventSetActivePlayer, "set_active_player"},
	{eventSetActivePhase, "set_active_phase"},
	{eventDumpZone, "dump_zone"}
};

ServerEventData::ServerEventData(const QString &line)
{
	QStringList values = line.split("|");

	IsPublic = !values.takeFirst().compare("public");
	PlayerId = values.takeFirst().toInt();
	PlayerName = values.takeFirst();

	QString type = values.takeFirst();
	bool found = false;
	for (int i = 0; i < event_count; i++)
		if (!type.compare(event_strings[i].str)) {
			EventType = event_strings[i].type;
			found = true;
			break;
		}
	if (!found)
		EventType = eventInvalid;

	EventData = values;
}
