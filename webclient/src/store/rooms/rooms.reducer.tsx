import * as _ from "lodash";

import { GameSortField, UserSortField, SortDirection } from "types";

import { SortUtil } from "../common";

import { RoomsState } from "./rooms.interfaces"
import { MAX_ROOM_MESSAGES, Types } from "./rooms.types";

const initialState: RoomsState = {
  rooms: {},
  joined: {},
  messages: {},
  sortGamesBy: {
    field: GameSortField.START_TIME,
    order: SortDirection.DESC
  },
  sortUsersBy: {
    field: UserSortField.NAME,
    order: SortDirection.ASC
  }
};

export const roomsReducer = (state = initialState, action: any) => {
  switch(action.type) {
    case Types.CLEAR_STORE: {
      return {
        ...initialState
      };
    }
    case Types.UPDATE_ROOMS: {
      const rooms = {
        ...state.rooms
      };

      // Server does not send everything on updates
      _.each(action.rooms, (room, order) => {
        const { roomId } = room;
        const existing = rooms[roomId] || {};
        
        const update = { ...room };
        delete update.gameList;
        delete update.gametypeList;
        delete update.userList;

        rooms[roomId] = {
          ...existing,
          ...update,
          order
        };
      });

      return { ...state, rooms };
    }
    case Types.JOIN_ROOM: {
      const { roomInfo } = action;
      const { joined, rooms, sortGamesBy, sortUsersBy } = state;

      const { roomId } = roomInfo;

      const gameList = [
        ...roomInfo.gameList
      ];

      const userList = [
        ...roomInfo.userList
      ];

      SortUtil.sortByField(gameList, sortGamesBy);
      SortUtil.sortUsersByField(userList, sortUsersBy);

      return {
        ...state,

        rooms: {
          ...rooms,
          [roomId]: {
            ...roomInfo,
            gameList,
            userList
          }
        },

        joined: {
          ...joined,
          [roomId]: true
        },
      }
    }
    case Types.LEAVE_ROOM: {
      const { roomId } = action;
      const { joined, messages } = state;

      const _joined = {
        ...joined
      };

      const _messages = {
        ...messages
      };

      delete _joined[roomId];
      delete _messages[roomId];

      return {
        ...state,

        joined: _joined,
        messages: _messages,
      }
    }
    case Types.ADD_MESSAGE: {
      const { roomId, message } = action;
      const { messages } = state;

      let roomMessages = [ ...(messages[roomId] || []) ];

      if (roomMessages.length === MAX_ROOM_MESSAGES) {
        roomMessages.shift();
      }

      message.timeReceived = new Date().getTime();
      roomMessages.push(message);

      return {
        ...state,
        messages: {
          ...messages,

          [roomId]: [
            ...roomMessages
          ]
        }
      }
    }
    // @TODO improve this reducer, likely by improving the store model
    case Types.UPDATE_GAMES: {
      const { roomId, games } = action;
      const { rooms, sortGamesBy } = state;
      const room = rooms[roomId];

      if (!room) {
        return { ...state };
      }

      // Create map of games with update objects
      const toUpdate = games.reduce((map, game) => {
        map[game.gameId] = game;
        return map;
      }, {});

      const gameUpdates = room.gameList
        // filter out closed games and remove from update map
        .filter(game => {
          const gameUpdate = toUpdate[game.gameId];
          const closedGame = gameUpdate && gameUpdate.closed;

          if (closedGame) {
            delete toUpdate[game.gameId];
          }

          return !closedGame;
        })
        .map(game => {
          const gameUpdate = toUpdate[game.gameId];

          if (gameUpdate) {
            delete toUpdate[game.gameId];

            return {
              ...game,
              ...gameUpdate
            };
          }

          return game;
        });

      // Push new games to end of list
      if (_.size(toUpdate)) {
        _.each(toUpdate, game => gameUpdates.push(game));
      }

      const gameList = [ ...gameUpdates ];

      SortUtil.sortByField(gameList, sortGamesBy);

      return {
        ...state,
        rooms: {
          ...rooms,
          [roomId]: {
            ...room,
            gameList
          }
        }
      }
    }
    case Types.USER_JOINED: {
      const { roomId, user } = action;
      const { rooms, sortUsersBy } = state;

      const room = { ...rooms[roomId] };

      const userList = [
        ...room.userList,
        user
      ];

      SortUtil.sortUsersByField(userList, sortUsersBy);

      return {
        ...state,
        rooms: {
          ...rooms,
          [roomId]: {
            ...room,
            userList
          } 
        }
      };
    }
    case Types.USER_LEFT: {
      const { roomId, name } = action;
      const { rooms } = state;

      const room = { ...rooms[roomId] };
      const userList = room.userList.filter(user =>  user.name !== name);

      return {
        ...state,
        rooms: {
          ...rooms,
          [roomId]: {
            ...room,
            userList
          } 
        }
      };
    }
    case Types.SORT_GAMES: {
      const { field, order, roomId } = action;
      const { rooms } = state;

      const gameList = [ ...rooms[roomId].gameList ];

      const sortGamesBy = {
        field, order
      };

      SortUtil.sortByField(gameList, sortGamesBy);

      return {
        ...state,

        rooms: {
          ...rooms,
          [roomId]: {
            ...rooms[roomId],
            gameList
          }
        },

        sortGamesBy
      }
    }
    default:
      return state;
  }
}
