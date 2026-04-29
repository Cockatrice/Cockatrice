import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { App, Data, Enriched } from '@app/types';

import { mergeSetFields, normalizeGameObject, normalizeGametypeMap, normalizeRoomInfo, normalizeUserMessage } from '../common';

import { GameFilters, RoomsState } from './rooms.interfaces';
import { DEFAULT_GAME_FILTERS } from './gameFilters';

export const MAX_ROOM_MESSAGES = 1000;

const initialState: RoomsState = {
  rooms: {},
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
  },
  selectedGameIds: {},
  gameFilters: {},
  joinGamePending: false,
  joinGameError: null,
};

export const roomsSlice = createSlice({
  name: 'rooms',
  initialState,
  reducers: {
    clearStore: () => initialState,

    updateRooms: (state, action: PayloadAction<{ rooms: Data.ServerInfo_Room[] }>) => {
      const { rooms } = action.payload;

      // @critical Partial merge — preserve normalized games/users maps AND
      // any fields the incoming ServerInfo_Room didn't set. The desktop server
      // broadcasts Event_ListRooms with only room_id/player_count/game_count
      // set on user join/leave (server_room.cpp addClient/removeClient); a
      // wholesale replacement would wipe name/description/permissions.
      rooms.forEach((rawRoom, order) => {
        const { roomId } = rawRoom;
        const existing = state.rooms[roomId];

        if (existing) {
          mergeSetFields(Data.ServerInfo_RoomSchema, existing.info, rawRoom);
          if (rawRoom.gametypeList.length > 0) {
            existing.gametypeMap = normalizeGametypeMap(rawRoom.gametypeList);
          }
          existing.order = order;
        } else {
          state.rooms[roomId] = {
            info: rawRoom,
            gametypeMap: normalizeGametypeMap(rawRoom.gametypeList),
            order,
            games: {},
            users: {},
          };
        }
      });
    },

    joinRoom: (state, action: PayloadAction<{ roomInfo: Data.ServerInfo_Room }>) => {
      const { roomInfo: rawRoomInfo } = action.payload;

      const roomEntry = normalizeRoomInfo(rawRoomInfo);
      const roomId = roomEntry.info.roomId;

      state.rooms[roomId] = roomEntry;
      state.joinedRoomIds[roomId] = true;
    },

    leaveRoom: (state, action: PayloadAction<{ roomId: number }>) => {
      const { roomId } = action.payload;

      delete state.joinedRoomIds[roomId];
      delete state.joinedGameIds[roomId];
      delete state.messages[roomId];
      delete state.selectedGameIds[roomId];
      delete state.gameFilters[roomId];

      const room = state.rooms[roomId];
      if (room) {
        room.games = {};
        room.users = {};
      }
    },

    addMessage: (state, action: PayloadAction<{ roomId: number; message: Enriched.Message }>) => {
      const { roomId, message } = action.payload;

      if (!state.messages[roomId]) {
        state.messages[roomId] = [];
      }
      const msgs = state.messages[roomId];
      if (msgs.length >= MAX_ROOM_MESSAGES) {
        state.messages[roomId] = msgs.slice(msgs.length - MAX_ROOM_MESSAGES + 1);
      }
      state.messages[roomId].push(normalizeUserMessage(message));
    },

    updateGames: (state, action: PayloadAction<{ roomId: number; games: Data.ServerInfo_Game[] }>) => {
      const { roomId, games } = action.payload;
      const room = state.rooms[roomId];

      if (!room || !games?.length) {
        return;
      }

      const gametypeMap = room.gametypeMap ?? {};

      for (const rawGame of games) {
        if (rawGame.closed) {
          delete room.games[rawGame.gameId];
          if (state.selectedGameIds[roomId] === rawGame.gameId) {
            state.selectedGameIds[roomId] = undefined;
          }
          continue;
        }
        const existing = room.games[rawGame.gameId];
        if (existing) {
          const merged: Data.ServerInfo_Game = { ...existing.info, ...rawGame };
          room.games[rawGame.gameId] = {
            info: merged,
            gameType: merged.gameTypes?.length
              ? (gametypeMap[merged.gameTypes[0]] ?? '')
              : existing.gameType,
          };
        } else {
          room.games[rawGame.gameId] = normalizeGameObject(rawGame, gametypeMap);
        }
      }
    },

    userJoined: (state, action: PayloadAction<{ roomId: number; user: Data.ServerInfo_User }>) => {
      const { roomId, user } = action.payload;
      const room = state.rooms[roomId];
      if (!room) {
        return;
      }

      room.users[user.name] = user;
    },

    userLeft: (state, action: PayloadAction<{ roomId: number; name: string }>) => {
      const { roomId, name } = action.payload;
      const room = state.rooms[roomId];
      if (!room) {
        return;
      }

      delete room.users[name];
    },

    sortGames: (state, action: PayloadAction<{ roomId: number; field: App.GameSortField; order: App.SortDirection }>) => {
      const { field, order } = action.payload;
      state.sortGamesBy = { field, order };
    },

    removeMessages: (state, action: PayloadAction<{ roomId: number; name: string; amount: number }>) => {
      const { name, amount, roomId } = action.payload;
      const roomMessages = state.messages[roomId];

      if (!roomMessages) {
        return;
      }

      const prefix = `${name}:`;
      const keep = new Array(roomMessages.length).fill(true);
      let remaining = amount;
      for (let i = roomMessages.length - 1; i >= 0 && remaining > 0; i--) {
        if (roomMessages[i].message.indexOf(prefix) === 0) {
          keep[i] = false;
          remaining--;
        }
      }

      state.messages[roomId] = roomMessages.filter((_, i) => keep[i]);
    },

    joinedGame: (state, action: PayloadAction<{ gameId: number; roomId: number }>) => {
      const { gameId, roomId } = action.payload;

      if (!state.joinedGameIds[roomId]) {
        state.joinedGameIds[roomId] = {};
      }
      state.joinedGameIds[roomId][gameId] = true;
    },

    selectGame: (state, action: PayloadAction<{ roomId: number; gameId: number | undefined }>) => {
      const { roomId, gameId } = action.payload;
      state.selectedGameIds[roomId] = gameId;
    },

    setGameFilters: (state, action: PayloadAction<{ roomId: number; filters: GameFilters }>) => {
      const { roomId, filters } = action.payload;
      state.gameFilters[roomId] = filters;
    },

    clearGameFilters: (state, action: PayloadAction<{ roomId: number }>) => {
      const { roomId } = action.payload;
      state.gameFilters[roomId] = { ...DEFAULT_GAME_FILTERS };
    },

    setJoinGamePending: (state, action: PayloadAction<{ pending: boolean }>) => {
      state.joinGamePending = action.payload.pending;
    },

    setJoinGameError: (state, action: PayloadAction<{ code: number; message: string }>) => {
      state.joinGameError = { code: action.payload.code, message: action.payload.message };
      state.joinGamePending = false;
    },

    clearJoinGameError: (state) => {
      state.joinGameError = null;
    },
  },
});

export const roomsReducer = roomsSlice.reducer;
