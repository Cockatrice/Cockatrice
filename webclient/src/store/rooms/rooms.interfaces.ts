import { App, Enriched } from '@app/types';

export interface RoomsState {
  rooms: RoomsStateRooms;
  joinedRoomIds: JoinedRooms;
  joinedGameIds: JoinedGames;
  messages: RoomsStateMessages;
  sortGamesBy: RoomsStateSortGamesBy;
  sortUsersBy: RoomsStateSortUsersBy;
  selectedGameIds: SelectedGameIds;
  gameFilters: RoomsStateGameFilters;
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

export interface SelectedGameIds {
  [roomId: number]: number | undefined;
}

export interface RoomsStateGameFilters {
  [roomId: number]: GameFilters;
}

/**
 * Mirrors desktop GamesProxyModel filter parameters
 * (cockatrice/src/interface/widgets/server/games_model.cpp).
 * `gameTypeFilter` is the set of allowed game-type ids; empty = no restriction.
 * `creatorNameFilters` is a list of comma-separated substrings; empty = no restriction.
 */
export interface GameFilters {
  hideBuddiesOnlyGames: boolean;
  hideIgnoredUserGames: boolean;
  hideFullGames: boolean;
  hideGamesThatStarted: boolean;
  hidePasswordProtectedGames: boolean;
  hideNotBuddyCreatedGames: boolean;
  hideOpenDecklistGames: boolean;
  gameNameFilter: string;
  creatorNameFilters: string[];
  gameTypeFilter: number[];
  maxPlayersFilterMin: number;
  maxPlayersFilterMax: number;
  maxGameAgeSeconds: number;
  showOnlyIfSpectatorsCanWatch: boolean;
  showSpectatorPasswordProtected: boolean;
  showOnlyIfSpectatorsCanChat: boolean;
  showOnlyIfSpectatorsCanSeeHands: boolean;
}
