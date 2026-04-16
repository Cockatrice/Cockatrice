import { createSelector } from '@reduxjs/toolkit';
import { Data, Enriched } from '@app/types';
import { SortUtil } from '../common';
import { RoomsState } from './rooms.interfaces';

interface State {
  rooms: RoomsState
}

const EMPTY_GAMES: Enriched.Game[] = [];
const EMPTY_USERS: Data.ServerInfo_User[] = [];
const EMPTY_GAMES_MAP: { [id: number]: Enriched.Game } = {};
const EMPTY_USERS_MAP: { [name: string]: Data.ServerInfo_User } = {};

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
}
