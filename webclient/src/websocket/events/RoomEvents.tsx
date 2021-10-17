import { Game, Message, User } from 'types';
import { RoomPersistence } from '../persistence/RoomPersistence';
import { ProtobufEvents } from '../services/ProtobufService';

export const RoomEvents: ProtobufEvents = {
  ".Event_JoinRoom.ext": joinRoom,
  ".Event_LeaveRoom.ext": leaveRoom,
  ".Event_ListGames.ext": listGames,
  ".Event_RoomSay.ext": roomSay,
};

function joinRoom({ userInfo }: JoinRoomData, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;

  RoomPersistence.userJoined(roomId, userInfo);
}

function leaveRoom({ name }: LeaveRoomData, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;
  RoomPersistence.userLeft(roomId, name);
}

function listGames({ gameList }: ListGamesData, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;
  RoomPersistence.updateGames(roomId, gameList);
}

function roomSay(message: Message, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;
  RoomPersistence.addMessage(roomId, message);
}

interface RoomEvent {
  roomEvent: {
    roomId: number;
  }
}

interface JoinRoomData {
  userInfo: User;
}

interface LeaveRoomData {
  name: string;
}

interface ListGamesData {
  gameList: Game[];
}
