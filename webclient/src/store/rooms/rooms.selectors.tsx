import * as _ from 'lodash';
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

  getJoinedRooms: (state: State) => {
    const joined = Selectors.getJoinedRoomIds(state);
    return _.filter(Selectors.getRooms(state), room => joined[room.roomId]);
  },

  getJoinedGames: (state: State, roomId: number) => {
    const joined = Selectors.getJoinedGameIds(state)[roomId];
    return _.filter(Selectors.getGames(state)[roomId], game => joined[game.gameId]);
  },

  getRoomMessages: (state: State, roomId: number) => Selectors.getMessages(state)[roomId],
  getRoomGames: (state: State, roomId: number) => Selectors.getRooms(state)[roomId].gameList,
  getRoomUsers: (state: State, roomId: number) => Selectors.getRooms(state)[roomId].userList

}

