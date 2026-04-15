import * as _ from 'lodash';

import { App, Enriched } from '@app/types';

import { normalizeGameObject, normalizeGametypeMap, normalizeRoomInfo, normalizeUserMessage, SortUtil } from '../common';

import { RoomsAction } from './rooms.actions';
import { RoomsState } from './rooms.interfaces'
import { MAX_ROOM_MESSAGES, Types } from './rooms.types';

const initialState: RoomsState = {
  rooms: {},
  games: {},
  joinedRoomIds: {},
  joinedGameIds: {},
  messages: {},
  sortGamesBy: {
    field: App.GameSortField.START_TIME,
    order: App.SortDirection.DESC
  },
  sortUsersBy: {
    field: App.UserSortField.NAME,
    order: App.SortDirection.ASC
  }
};

export const roomsReducer = (state = initialState, action: RoomsAction) => {
  switch (action.type) {
    case Types.CLEAR_STORE: {
      return {
        ...initialState
      };
    }

    case Types.UPDATE_ROOMS: {
      const rooms = {
        ...state.rooms
      };

      // Server does not send everything on updates — preserve existing gameList/userList
      _.each(action.rooms, (rawRoom, order) => {
        const { gameList: _g, gametypeList, userList: _u, ...roomMeta } = rawRoom;
        const { roomId } = roomMeta;
        const existing = rooms[roomId] || {};

        const gametypeMap = normalizeGametypeMap(gametypeList);

        rooms[roomId] = {
          ...(existing as Enriched.Room),
          ...roomMeta,
          gametypeMap,
          gameList: (existing as Enriched.Room).gameList,
          userList: (existing as Enriched.Room).userList,
          order,
        };
      });

      return { ...state, rooms };
    }

    case Types.JOIN_ROOM: {
      const { roomInfo: rawRoomInfo } = action;
      const { joinedRoomIds, rooms, sortGamesBy, sortUsersBy } = state;

      const roomInfo = normalizeRoomInfo(rawRoomInfo);
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

        joinedRoomIds: {
          ...joinedRoomIds,
          [roomId]: true
        },
      }
    }

    case Types.LEAVE_ROOM: {
      const { roomId } = action;
      const { joinedRoomIds, messages } = state;

      const _joined = {
        ...joinedRoomIds
      };

      const _messages = {
        ...messages
      };

      delete _joined[roomId];
      delete _messages[roomId];

      return {
        ...state,

        joinedRoomIds: _joined,
        messages: _messages,
      }
    }

    case Types.ADD_MESSAGE: {
      const { roomId, message } = action;
      const { messages } = state;

      let roomMessages = [...(messages[roomId] || [])];

      if (roomMessages.length === MAX_ROOM_MESSAGES) {
        roomMessages.shift();
      }

      const normalized = normalizeUserMessage({ ...message, timeReceived: Date.now() });
      roomMessages.push(normalized);

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

      // An empty gameList means no game updates — skip to avoid
      // overwriting the existing game list with an empty one.
      if (!room || !games?.length) {
        return state;
      }

      // Normalize incoming raw proto games using the room's gametypeMap
      const gametypeMap = room.gametypeMap ?? {};
      const normalizedGames = games.map(g => normalizeGameObject(g, gametypeMap));

      // Create map of games with update objects
      const toUpdate = normalizedGames.reduce((map, game) => {
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

      const gameList = [...gameUpdates];

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
      const userList = room.userList.filter(user => user.name !== name);

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

      const gameList = [...rooms[roomId].gameList];

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

    case Types.REMOVE_MESSAGES: {
      const { name, amount, roomId } = action;
      const { messages } = state;
      let amountRemoved = 0;

      return {
        ...state,
        messages: {
          ...messages,
          [roomId]: messages[roomId]
            .reverse()
            .filter(({ message }) => {
              if (amount === amountRemoved) {
                return true;
              }

              const keep = message.indexOf(`${name}:`) !== 0;

              if (!keep) {
                amountRemoved++;
              }

              return keep;
            })
            .reverse()
        }
      }
    }

    case Types.JOINED_GAME: {
      const { gameId, roomId } = action;
      const { joinedGameIds } = state;

      return {
        ...state,
        joinedGameIds: {
          ...joinedGameIds,
          [roomId]: {
            ...joinedGameIds[roomId],
            [gameId]: true,
          }
        }
      }
    }

    // Signal-only — no state mutation needed; explicit for discriminated-union exhaustiveness
    case Types.GAME_CREATED:
      return state;

    default:
      return state;
  }
}
