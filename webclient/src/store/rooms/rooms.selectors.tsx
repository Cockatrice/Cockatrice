import * as _ from 'lodash';
import { createSelector } from '@reduxjs/toolkit';
import { RoomsState } from './rooms.interfaces';

interface State {
  rooms: RoomsState
}

export const Selectors = {
  getRooms: ({ rooms }: State) => rooms.rooms,
  getGames: ({ rooms }: State) => rooms.games,
  getRoom: ({ rooms }: State, id: number) =>
    _.find(rooms.rooms, ({ roomId }) => roomId === id),
  getJoinedRoomIds: ({ rooms }: State) => rooms.joinedRoomIds,
  getJoinedGameIds: ({ rooms }: State) => rooms.joinedGameIds,
  getMessages: ({ rooms }: State) => rooms.messages,
  getSortGamesBy: ({ rooms: { sortGamesBy } }: State) => sortGamesBy,
  getSortUsersBy: ({ rooms: { sortUsersBy } }: State) => sortUsersBy,

  getJoinedRooms: createSelector(
    [(state: State) => state.rooms.rooms, (state: State) => state.rooms.joinedRoomIds],
    (rooms, joined) => _.filter(rooms, room => joined[room.roomId])
  ),

  getJoinedGames: createSelector(
    [(state: State, roomId: number) => state.rooms.games[roomId], (state: State, roomId: number) => state.rooms.joinedGameIds[roomId]],
    (games, joined) => _.filter(games, game => joined[game.gameId])
  ),

  getRoomMessages: (state: State, roomId: number) => Selectors.getMessages(state)[roomId],
  getRoomGames: (state: State, roomId: number) => Selectors.getRooms(state)[roomId].gameList,
  getRoomUsers: (state: State, roomId: number) => Selectors.getRooms(state)[roomId].userList

}

