#ifndef SERVER_PLAYER_REFERENCE_H
#define SERVER_PLAYER_REFERENCE_H

class PlayerReference {
private:
	int roomId;
	int gameId;
	int playerId;
public:
	PlayerReference(int _roomId, int _gameId, int _playerId) : roomId(_roomId), gameId(_gameId), playerId(_playerId) { }
	int getRoomId() const { return roomId; }
	int getGameId() const { return gameId; }
	int getPlayerId() const { return playerId; }
	bool operator==(const PlayerReference &other) { return ((roomId == other.roomId) && (gameId == other.gameId) && (playerId == other.playerId)); }
};

#endif
