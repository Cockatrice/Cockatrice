import { Game, Message, User } from 'types';
import { RoomPersistence } from '../persistence/RoomPersistence';
import { ProtobufEvents } from '../services/ProtobufService';

export const RoomEvents: ProtobufEvents = {
  '.Event_JoinRoom.ext': joinRoom,
  '.Event_LeaveRoom.ext': leaveRoom,
  '.Event_ListGames.ext': listGames,
  '.Event_RoomSay.ext': roomSay,
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

export interface RoomEvent {
  roomEvent: {
    roomId: number;
  }
}

export interface JoinRoomData {
  userInfo: User;
}

export interface LeaveRoomData {
  name: string;
}

export interface ListGamesData {
  gameList: Game[];
}
