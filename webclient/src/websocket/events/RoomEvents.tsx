import { RoomPersistence } from '../persistence/RoomPersistence';

export const RoomEvents = {
  ".Event_JoinRoom.ext": joinRoom,
  ".Event_LeaveRoom.ext": leaveRoom,
  ".Event_ListGames.ext": listGames,
  ".Event_RoomSay.ext": roomSay,
};

function joinRoom({ userInfo }, { roomEvent }) {
  const { roomId } = roomEvent;
  RoomPersistence.userJoined(roomId, userInfo);
}

function leaveRoom({ name }, { roomEvent }) {
  const { roomId } = roomEvent;
  RoomPersistence.userLeft(roomId, name);
}

function listGames({ gameList }, { roomEvent }) {
  const { roomId } = roomEvent;
  RoomPersistence.updateGames(roomId, gameList);
}

function roomSay(message, { roomEvent }) {
  const { roomId } = roomEvent;
  RoomPersistence.addMessage(roomId, message);
}
