export interface RoomsState {
  rooms: RoomsStateRooms;
  joined: JoinedRooms;
  messages: RoomsStateMessages;
  active: number;
}

export interface RoomsStateRooms {
  [roomId: number]: Room;
}

export interface JoinedRooms {
  [roomId: number]: boolean;
}

export interface RoomsStateMessages {
  [roomId: number]: Message[];
}

export interface Room {
  autoJoin: boolean
  description: string;
  gameCount: number;
  gameList: any[];
  gametypeList: any[];
  name: string;
  permissionlevel: RoomsStateRoomAccessLevel;
  playerCount: number;
  privilegelevel: RoomsStateRoomAccessLevel;
  roomId: number;
  userList: any[];
  order: number;
}

export interface Message {
  message: string;
  messageType: number;
  timeOf?: number;
}

export enum RoomsStateRoomAccessLevel {
  "none"
}
