import { Room } from 'types';

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

export interface Message {
  message: string;
  messageType: number;
  timeOf?: number;
}
