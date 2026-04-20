import { createSelector } from '@reduxjs/toolkit';
import { Data, Enriched } from '@app/types';
import { SortUtil } from '../common';
import { GameFilters, RoomsState } from './rooms.interfaces';
import { ServerState } from '../server/server.interfaces';
import {
  DEFAULT_GAME_FILTERS,
  GameFilterContext,
  gameMatchesFilters,
  isGameFiltersAtDefaults,
} from './gameFilters';

interface State {
  rooms: RoomsState;
  server?: ServerState;
}

const EMPTY_GAMES: Enriched.Game[] = [];
const EMPTY_USERS: Data.ServerInfo_User[] = [];
const EMPTY_GAMES_MAP: { [id: number]: Enriched.Game } = {};
const EMPTY_USERS_MAP: { [name: string]: Data.ServerInfo_User } = {};

const ZERO_COUNTS = { visible: 0, total: 0 };

export const Selectors = {
  getRooms: ({ rooms }: State) => rooms.rooms,
  getRoom: ({ rooms }: State, id: number) => rooms.rooms[id],
  getJoinedRoomIds: ({ rooms }: State) => rooms.joinedRoomIds,
  getJoinedGameIds: ({ rooms }: State) => rooms.joinedGameIds,
  getMessages: ({ rooms }: State) => rooms.messages,
  getSortGamesBy: ({ rooms: { sortGamesBy } }: State) => sortGamesBy,
  getSortUsersBy: ({ rooms: { sortUsersBy } }: State) => sortUsersBy,

  getJoinedRooms: createSelector(
    [(state: State) => state.rooms.rooms, (state: State) => state.rooms.joinedRoomIds],
    (rooms, joined) => Object.values(rooms).filter(room => joined[room.info.roomId])
  ),

  /**
   * Returns games in the given room that the local client has joined.
   * Reads from the room's normalized `games` map — fixes the pre-existing
   * bug where this selector read from a never-populated top-level `games` field.
   */
  getJoinedGames: createSelector(
    [
      (state: State, roomId: number) => state.rooms.rooms[roomId]?.games,
      (state: State, roomId: number) => state.rooms.joinedGameIds[roomId],
    ],
    (games, joined): Enriched.Game[] => {
      if (!games || !joined) {
        return EMPTY_GAMES;
      }
      return Object.values(games).filter(game => joined[game.info.gameId]);
    }
  ),

  getRoomMessages: (state: State, roomId: number) => state.rooms.messages[roomId],

  /** Raw keyed games map for a room. For a sorted array, use `getSortedRoomGames`. */
  getRoomGames: (state: State, roomId: number) => state.rooms.rooms[roomId]?.games ?? EMPTY_GAMES_MAP,

  /** Raw keyed users map for a room. For a sorted array, use `getSortedRoomUsers`. */
  getRoomUsers: (state: State, roomId: number) => state.rooms.rooms[roomId]?.users ?? EMPTY_USERS_MAP,

  /**
   * Sorted array view of a room's games for display. Memoized by the input
   * references — recomputes only when the games map, gametypeMap, or sort
   * config actually change.
   */
  getSortedRoomGames: createSelector(
    [
      (state: State, roomId: number) => state.rooms.rooms[roomId]?.games,
      (state: State) => state.rooms.sortGamesBy,
    ],
    (games, sortBy): Enriched.Game[] => {
      if (!games) {
        return EMPTY_GAMES;
      }
      return SortUtil.sortedByField(Object.values(games), sortBy);
    }
  ),

  /** Sorted array view of a room's users for display. */
  getSortedRoomUsers: createSelector(
    [
      (state: State, roomId: number) => state.rooms.rooms[roomId]?.users,
      (state: State) => state.rooms.sortUsersBy,
    ],
    (users, sortBy): Data.ServerInfo_User[] => {
      if (!users) {
        return EMPTY_USERS;
      }
      return SortUtil.sortedUsersByField(Object.values(users), sortBy);
    }
  ),

  getSelectedGameId: (state: State, roomId: number): number | undefined =>
    state.rooms.selectedGameIds?.[roomId],

  /** Returns the room's filter state, or DEFAULT_GAME_FILTERS if unset. */
  getGameFilters: (state: State, roomId: number): GameFilters =>
    state.rooms.gameFilters?.[roomId] ?? DEFAULT_GAME_FILTERS,

  isGameFilterActive: (state: State, roomId: number): boolean => {
    const filters = state.rooms.gameFilters?.[roomId];
    if (!filters) {
      return false;
    }
    return !isGameFiltersAtDefaults(filters);
  },

  /**
   * Sorted + filter-applied view of a room's games for display. Filters
   * mirror desktop GamesProxyModel; buddy/ignore checks read from server.
   */
  getFilteredRoomGames: createSelector(
    [
      (state: State, roomId: number) => state.rooms.rooms[roomId]?.games,
      (state: State) => state.rooms.sortGamesBy,
      (state: State, roomId: number) => state.rooms.gameFilters?.[roomId],
      (state: State) => state.server?.user,
      (state: State) => state.server?.buddyList,
      (state: State) => state.server?.ignoreList,
    ],
    (games, sortBy, filters, user, buddyList, ignoreList): Enriched.Game[] => {
      if (!games) {
        return EMPTY_GAMES;
      }
      const sorted = SortUtil.sortedByField(Object.values(games), sortBy);
      if (!filters || isGameFiltersAtDefaults(filters)) {
        return sorted;
      }
      const ctx: GameFilterContext = {
        isOwnUserRegistered: user
          ? (user.userLevel & Data.ServerInfo_User_UserLevelFlag.IsRegistered) ===
            Data.ServerInfo_User_UserLevelFlag.IsRegistered
          : false,
        isUserBuddy: (name) => Boolean(buddyList?.[name]),
        isUserIgnored: (name) => Boolean(ignoreList?.[name]),
        nowSeconds: Math.floor(Date.now() / 1000),
      };
      return sorted.filter((game) => gameMatchesFilters(game, filters, ctx));
    }
  ),

  /** Visible (post-filter) and total counts for the games-shown title. */
  getRoomGameCounts: createSelector(
    [
      (state: State, roomId: number) => state.rooms.rooms[roomId]?.games,
      (state: State, roomId: number) => state.rooms.gameFilters?.[roomId],
      (state: State) => state.server?.user,
      (state: State) => state.server?.buddyList,
      (state: State) => state.server?.ignoreList,
    ],
    (games, filters, user, buddyList, ignoreList): { visible: number; total: number } => {
      if (!games) {
        return ZERO_COUNTS;
      }
      const all = Object.values(games);
      const total = all.length;
      if (!filters || isGameFiltersAtDefaults(filters)) {
        return { visible: total, total };
      }
      const ctx: GameFilterContext = {
        isOwnUserRegistered: user
          ? (user.userLevel & Data.ServerInfo_User_UserLevelFlag.IsRegistered) ===
            Data.ServerInfo_User_UserLevelFlag.IsRegistered
          : false,
        isUserBuddy: (name) => Boolean(buddyList?.[name]),
        isUserIgnored: (name) => Boolean(ignoreList?.[name]),
        nowSeconds: Math.floor(Date.now() / 1000),
      };
      const visible = all.reduce((n, game) => (gameMatchesFilters(game, filters, ctx) ? n + 1 : n), 0);
      return { visible, total };
    }
  ),
}
