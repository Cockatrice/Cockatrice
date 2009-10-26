#ifndef PROTOCOL_COMMANDS_H
#define PROTOCOL_COMMANDS_H

#include "protocol.h"

class Command_Ping : public Command {
	Q_OBJECT
private:
public:
	Command_Ping();
	static ProtocolItem *newItem() { return new Command_Ping; }
};
class Command_Login : public Command {
	Q_OBJECT
private:
	QString username;
	QString password;
public:
	Command_Login(const QString &_username = QString(), const QString &_password = QString());
	QString getUsername() const { return username; }
	QString getPassword() const { return password; }
	static ProtocolItem *newItem() { return new Command_Login; }
protected:
	void extractParameters();
};
class Command_ChatListChannels : public Command {
	Q_OBJECT
private:
public:
	Command_ChatListChannels();
	static ProtocolItem *newItem() { return new Command_ChatListChannels; }
};
class Command_ChatJoinChannel : public Command {
	Q_OBJECT
private:
	QString channel;
public:
	Command_ChatJoinChannel(const QString &_channel = QString());
	QString getChannel() const { return channel; }
	static ProtocolItem *newItem() { return new Command_ChatJoinChannel; }
protected:
	void extractParameters();
};
class Command_ChatLeaveChannel : public ChatCommand {
	Q_OBJECT
private:
public:
	Command_ChatLeaveChannel(const QString &_channel = QString());
	static ProtocolItem *newItem() { return new Command_ChatLeaveChannel; }
};
class Command_ChatSay : public ChatCommand {
	Q_OBJECT
private:
	QString message;
public:
	Command_ChatSay(const QString &_channel = QString(), const QString &_message = QString());
	QString getMessage() const { return message; }
	static ProtocolItem *newItem() { return new Command_ChatSay; }
protected:
	void extractParameters();
};
class Command_ListGames : public Command {
	Q_OBJECT
private:
public:
	Command_ListGames();
	static ProtocolItem *newItem() { return new Command_ListGames; }
};
class Command_CreateGame : public Command {
	Q_OBJECT
private:
	QString description;
	QString password;
	int maxPlayers;
	bool spectatorsAllowed;
public:
	Command_CreateGame(const QString &_description = QString(), const QString &_password = QString(), int _maxPlayers = -1, bool _spectatorsAllowed = false);
	QString getDescription() const { return description; }
	QString getPassword() const { return password; }
	int getMaxPlayers() const { return maxPlayers; }
	bool getSpectatorsAllowed() const { return spectatorsAllowed; }
	static ProtocolItem *newItem() { return new Command_CreateGame; }
protected:
	void extractParameters();
};
class Command_JoinGame : public Command {
	Q_OBJECT
private:
	int gameId;
	QString password;
	bool spectator;
public:
	Command_JoinGame(int _gameId = -1, const QString &_password = QString(), bool _spectator = false);
	int getGameId() const { return gameId; }
	QString getPassword() const { return password; }
	bool getSpectator() const { return spectator; }
	static ProtocolItem *newItem() { return new Command_JoinGame; }
protected:
	void extractParameters();
};
class Command_LeaveGame : public GameCommand {
	Q_OBJECT
private:
public:
	Command_LeaveGame(int _gameId = -1);
	static ProtocolItem *newItem() { return new Command_LeaveGame; }
};
class Command_Say : public GameCommand {
	Q_OBJECT
private:
	QString message;
public:
	Command_Say(int _gameId = -1, const QString &_message = QString());
	QString getMessage() const { return message; }
	static ProtocolItem *newItem() { return new Command_Say; }
protected:
	void extractParameters();
};
class Command_Shuffle : public GameCommand {
	Q_OBJECT
private:
public:
	Command_Shuffle(int _gameId = -1);
	static ProtocolItem *newItem() { return new Command_Shuffle; }
};
class Command_RollDie : public GameCommand {
	Q_OBJECT
private:
	int sides;
public:
	Command_RollDie(int _gameId = -1, int _sides = -1);
	int getSides() const { return sides; }
	static ProtocolItem *newItem() { return new Command_RollDie; }
protected:
	void extractParameters();
};
class Command_DrawCards : public GameCommand {
	Q_OBJECT
private:
	int number;
public:
	Command_DrawCards(int _gameId = -1, int _number = -1);
	int getNumber() const { return number; }
	static ProtocolItem *newItem() { return new Command_DrawCards; }
protected:
	void extractParameters();
};
class Command_MoveCard : public GameCommand {
	Q_OBJECT
private:
	QString startZone;
	int cardId;
	QString targetZone;
	int x;
	int y;
	bool faceDown;
public:
	Command_MoveCard(int _gameId = -1, const QString &_startZone = QString(), int _cardId = -1, const QString &_targetZone = QString(), int _x = -1, int _y = -1, bool _faceDown = false);
	QString getStartZone() const { return startZone; }
	int getCardId() const { return cardId; }
	QString getTargetZone() const { return targetZone; }
	int getX() const { return x; }
	int getY() const { return y; }
	bool getFaceDown() const { return faceDown; }
	static ProtocolItem *newItem() { return new Command_MoveCard; }
protected:
	void extractParameters();
};
class Command_CreateToken : public GameCommand {
	Q_OBJECT
private:
	QString zone;
	QString name;
	QString pt;
	int x;
	int y;
public:
	Command_CreateToken(int _gameId = -1, const QString &_zone = QString(), const QString &_name = QString(), const QString &_pt = QString(), int _x = -1, int _y = -1);
	QString getZone() const { return zone; }
	QString getName() const { return name; }
	QString getPt() const { return pt; }
	int getX() const { return x; }
	int getY() const { return y; }
	static ProtocolItem *newItem() { return new Command_CreateToken; }
protected:
	void extractParameters();
};
class Command_CreateArrow : public GameCommand {
	Q_OBJECT
private:
	int startPlayerId;
	QString startZone;
	int startCardId;
	int targetPlayerId;
	QString targetPlayerZone;
	int targetCardId;
	int color;
public:
	Command_CreateArrow(int _gameId = -1, int _startPlayerId = -1, const QString &_startZone = QString(), int _startCardId = -1, int _targetPlayerId = -1, const QString &_targetPlayerZone = QString(), int _targetCardId = -1, int _color = -1);
	int getStartPlayerId() const { return startPlayerId; }
	QString getStartZone() const { return startZone; }
	int getStartCardId() const { return startCardId; }
	int getTargetPlayerId() const { return targetPlayerId; }
	QString getTargetPlayerZone() const { return targetPlayerZone; }
	int getTargetCardId() const { return targetCardId; }
	int getColor() const { return color; }
	static ProtocolItem *newItem() { return new Command_CreateArrow; }
protected:
	void extractParameters();
};
class Command_DeleteArrow : public GameCommand {
	Q_OBJECT
private:
	int arrowId;
public:
	Command_DeleteArrow(int _gameId = -1, int _arrowId = -1);
	int getArrowId() const { return arrowId; }
	static ProtocolItem *newItem() { return new Command_DeleteArrow; }
protected:
	void extractParameters();
};
class Command_SetCardAttr : public GameCommand {
	Q_OBJECT
private:
	QString zone;
	int cardId;
	QString attrName;
	QString attrValue;
public:
	Command_SetCardAttr(int _gameId = -1, const QString &_zone = QString(), int _cardId = -1, const QString &_attrName = QString(), const QString &_attrValue = QString());
	QString getZone() const { return zone; }
	int getCardId() const { return cardId; }
	QString getAttrName() const { return attrName; }
	QString getAttrValue() const { return attrValue; }
	static ProtocolItem *newItem() { return new Command_SetCardAttr; }
protected:
	void extractParameters();
};
class Command_ReadyStart : public GameCommand {
	Q_OBJECT
private:
public:
	Command_ReadyStart(int _gameId = -1);
	static ProtocolItem *newItem() { return new Command_ReadyStart; }
};
class Command_IncCounter : public GameCommand {
	Q_OBJECT
private:
	int counterId;
	int delta;
public:
	Command_IncCounter(int _gameId = -1, int _counterId = -1, int _delta = -1);
	int getCounterId() const { return counterId; }
	int getDelta() const { return delta; }
	static ProtocolItem *newItem() { return new Command_IncCounter; }
protected:
	void extractParameters();
};
class Command_AddCounter : public GameCommand {
	Q_OBJECT
private:
	QString counterName;
	int color;
	int radius;
	int value;
public:
	Command_AddCounter(int _gameId = -1, const QString &_counterName = QString(), int _color = -1, int _radius = -1, int _value = -1);
	QString getCounterName() const { return counterName; }
	int getColor() const { return color; }
	int getRadius() const { return radius; }
	int getValue() const { return value; }
	static ProtocolItem *newItem() { return new Command_AddCounter; }
protected:
	void extractParameters();
};
class Command_SetCounter : public GameCommand {
	Q_OBJECT
private:
	int counterId;
	int value;
public:
	Command_SetCounter(int _gameId = -1, int _counterId = -1, int _value = -1);
	int getCounterId() const { return counterId; }
	int getValue() const { return value; }
	static ProtocolItem *newItem() { return new Command_SetCounter; }
protected:
	void extractParameters();
};
class Command_DelCounter : public GameCommand {
	Q_OBJECT
private:
	int counterId;
public:
	Command_DelCounter(int _gameId = -1, int _counterId = -1);
	int getCounterId() const { return counterId; }
	static ProtocolItem *newItem() { return new Command_DelCounter; }
protected:
	void extractParameters();
};
class Command_NextTurn : public GameCommand {
	Q_OBJECT
private:
public:
	Command_NextTurn(int _gameId = -1);
	static ProtocolItem *newItem() { return new Command_NextTurn; }
};
class Command_SetActivePhase : public GameCommand {
	Q_OBJECT
private:
	int phase;
public:
	Command_SetActivePhase(int _gameId = -1, int _phase = -1);
	int getPhase() const { return phase; }
	static ProtocolItem *newItem() { return new Command_SetActivePhase; }
protected:
	void extractParameters();
};
class Command_DumpZone : public GameCommand {
	Q_OBJECT
private:
	int playerId;
	QString zoneName;
	int numberCards;
public:
	Command_DumpZone(int _gameId = -1, int _playerId = -1, const QString &_zoneName = QString(), int _numberCards = -1);
	int getPlayerId() const { return playerId; }
	QString getZoneName() const { return zoneName; }
	int getNumberCards() const { return numberCards; }
	static ProtocolItem *newItem() { return new Command_DumpZone; }
protected:
	void extractParameters();
};
class Command_StopDumpZone : public GameCommand {
	Q_OBJECT
private:
	int playerId;
	QString zoneName;
public:
	Command_StopDumpZone(int _gameId = -1, int _playerId = -1, const QString &_zoneName = QString());
	int getPlayerId() const { return playerId; }
	QString getZoneName() const { return zoneName; }
	static ProtocolItem *newItem() { return new Command_StopDumpZone; }
protected:
	void extractParameters();
};
class Command_DumpAll : public GameCommand {
	Q_OBJECT
private:
public:
	Command_DumpAll(int _gameId = -1);
	static ProtocolItem *newItem() { return new Command_DumpAll; }
};
class Command_SubmitDeck : public GameCommand {
	Q_OBJECT
private:
public:
	Command_SubmitDeck(int _gameId = -1);
	static ProtocolItem *newItem() { return new Command_SubmitDeck; }
};

#endif
