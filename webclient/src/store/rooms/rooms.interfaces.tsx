import { App, Enriched } from '@app/types';

export interface RoomsState {
  rooms: RoomsStateRooms;
  joinedRoomIds: JoinedRooms;
  joinedGameIds: JoinedGames;
  messages: RoomsStateMessages;
  sortGamesBy: RoomsStateSortGamesBy;
  sortUsersBy: RoomsStateSortUsersBy;
}

export interface RoomsStateRooms {
  [roomId: number]: Enriched.Room;
}

export interface JoinedRooms {
  [roomId: number]: boolean;
}

export interface JoinedGames {
  [roomId: number]: {
    [gameId: number]: boolean;
  };
}

export interface RoomsStateMessages {
  [roomId: number]: Enriched.Message[];
}

export interface RoomsStateSortGamesBy extends App.SortBy {
  field: App.GameSortField
}

export interface RoomsStateSortUsersBy extends App.SortBy {
  field: App.UserSortField
}
