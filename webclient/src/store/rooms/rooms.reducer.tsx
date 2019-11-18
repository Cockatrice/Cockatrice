import * as _ from 'lodash';

import { RoomsState } from './rooms.interfaces'
import { Types } from './rooms.types';

const initialState: RoomsState = {
  rooms: {},
  joined: {},
  messages: {},
  active: null,
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

      // Server does not send everything everytime
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
      const { roomId } = roomInfo;

      const { joined, rooms } = state;

      roomInfo.userList.sort((a, b) => a.name.localeCompare(b.name));

      return {
        ...state,

        rooms: {
          ...rooms,
          [roomId]: roomInfo
        },

        joined: {
          ...joined,
          [roomId]: true
        },

        active: roomId
      }
    }
    case Types.LEAVE_ROOM: {
      const { roomId } = action;
      const { joined, messages, active } = state;

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

        active: active === roomId ? null : active
      }
    }
    case Types.ADD_MESSAGE: {
      const { roomId, message } = action;
      const { messages } = state;

      let roomMessages = [ ...(messages[roomId] || []) ];

      // @TODO add this value to a const somewhere higher up
      if (roomMessages.length === 1000) {
        roomMessages.shift();
      }

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
    case Types.UPDATE_GAMES: {
      const { roomId, games } = action;
      const room = state.rooms[roomId];

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

      return {
        ...state,
        rooms: {
          ...state.rooms,
          [roomId]: {
            ...room,
            gameList: [
              ...gameUpdates
            ]
          }
        }
      }
    }
    case Types.USER_JOINED: {
      const { roomId, user } = action;

      const room = { ...state.rooms[roomId] };

      room.userList = [
        ...room.userList,
        user
      ];

      room.userList.sort((a, b) => a.name.localeCompare(b.name));

      return {
        ...state,
        rooms: {
          ...state.rooms,
          [roomId]: room 
        }
      };
    }
    case Types.USER_LEFT: {
      const { roomId, name } = action;

      const room = { ...state.rooms[roomId] };
      room.userList = room.userList.filter(user =>  user.name !== name);

      return {
        ...state,
        rooms: {
          ...state.rooms,
          [roomId]: room 
        }
      };
    }
    default:
      return state;
  }
}
