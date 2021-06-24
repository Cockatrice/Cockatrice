import * as _ from "lodash";
import { RoomsState } from "./rooms.interfaces";

interface State {
  rooms: RoomsState
}

export const Selectors = {
  getRooms: ({ rooms }: State) => rooms.rooms,
  getRoom: ({ rooms }: State, id: number) =>
    _.find(rooms.rooms, ({roomId}) => roomId === id),
  getJoined: ({ rooms }: State) => rooms.joined,
  getMessages: ({ rooms }: State) => rooms.messages,
  getSortGamesBy: ({ rooms: { sortGamesBy } }: State) => sortGamesBy,
  getSortUsersBy: ({ rooms: { sortUsersBy } }: State) => sortUsersBy,

  getJoinedRooms: (state: State) => {
    const joined = Selectors.getJoined(state);
    return _.filter(Selectors.getRooms(state), room => joined[room.roomId]);
  },
  getRoomMessages: (state: State, roomId: number) => Selectors.getMessages(state)[roomId],
  getRoomGames: (state: State, roomId: number) => Selectors.getRooms(state)[roomId].gameList,
  getRoomUsers: (state: State, roomId: number) => Selectors.getRooms(state)[roomId].userList

}

