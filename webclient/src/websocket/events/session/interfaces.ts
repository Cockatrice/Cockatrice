import { Game, NotificationType, Room, User } from 'types';

export interface AddToListData {
  listName: string;
  userInfo: User;
}

export interface ConnectionClosedData {
  endTime: number;
  reason: number;
  reasonStr: string;
}

export interface GameJoinedData {
  gameInfo: Game;
  playerId: number;
  spectator: boolean;
  resuming: boolean;
  judge: boolean;
}

export interface ListRoomsData {
  roomList: Room[];
}

export interface NotifyUserData {
  type: NotificationType;
  warningReason: string;
  customTitle: string;
  customContent: string;
}

export interface PlayerGamePropertiesData {
  playerId: number;
  userInfo: User;
  spectator: boolean;
  conceded: boolean;
  readyStart: boolean;
  deckHash: string;
  pingSeconds: number;
  sideboardLocked: boolean;
  judge: boolean;
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

export interface ServerShutdownData {
  reason: string;
  minutes: number;
}

export interface UserJoinedData {
  userInfo: User;
}

export interface UserLeftData {
  name: string;
}

export interface UserMessageData {
  senderName: string;
  receiverName: string;
  message: string;
}
