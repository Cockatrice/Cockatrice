import { Game, Room, User } from 'types';

export interface SessionEvent {
  sessionEvent: {}
}

export interface AddToListData {
  listName: string;
  userInfo: User;
}

export interface ConnectionClosedData {
  endTime: number;
  reason: number;
  reasonStr: string;
}

export interface ListRoomsData {
  roomList: Room[];
}

export interface RemoveFromListData {
  listName: string;
  userName: string;
}

export interface ServerIdentificationData {
  protocolVersion: number;
  serverName: string;
  serverVersion: string;
  serverOptions: number;
}

export interface ServerMessageData {
  message: string;
}

export interface UserJoinedData {
  userInfo: User;
}

export interface UserLeftData {
  name: string;
}

export interface GameJoinedData {
  gameInfo: Game;
  playerId: number;
  spectator: boolean;
  resuming: boolean;
  judge: boolean;
}
