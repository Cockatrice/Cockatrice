import { GameSortField, Room, SortBy, UserSortField } from "types";

export interface RoomsState {
  rooms: RoomsStateRooms;
  joined: JoinedRooms;
  messages: RoomsStateMessages;
  sortGamesBy: RoomsStateSortGamesBy;
  sortUsersBy: RoomsStateSortUsersBy;
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

export interface RoomsStateSortGamesBy extends SortBy {
  field: GameSortField
}

export interface RoomsStateSortUsersBy extends SortBy {
  field: UserSortField
}

export interface Message {
  message: string;
  messageType: number;
  timeReceived: number;
  timeOf?: number;
}
