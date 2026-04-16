import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { App, Data, Enriched } from '@app/types';

import { normalizeGameObject, normalizeGametypeMap, normalizeRoomInfo, normalizeUserMessage } from '../common';

import { RoomsState } from './rooms.interfaces'

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
  }
};

export const roomsSlice = createSlice({
  name: 'rooms',
  initialState,
  reducers: {
    clearStore: () => initialState,

    updateRooms: (state, action: PayloadAction<{ rooms: Data.ServerInfo_Room[] }>) => {
      const { rooms } = action.payload;

      // UPDATE_ROOMS carries metadata only. For existing rooms, replace
      // `info`, `gametypeMap` and `order`; preserve the normalized `games`
      // and `users` maps (those are maintained by their own events).
      rooms.forEach((rawRoom, order) => {
        const { roomId } = rawRoom;
        const existing = state.rooms[roomId];
        const gametypeMap = normalizeGametypeMap(rawRoom.gametypeList);

        if (existing) {
          existing.info = rawRoom;
          existing.gametypeMap = gametypeMap;
          existing.order = order;
        } else {
          state.rooms[roomId] = {
            info: rawRoom,
            gametypeMap,
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

      const room = state.rooms[roomId];
      if (room) {
        room.games = {};
        room.users = {};
      }
    },

    addMessage: (state, action: PayloadAction<{ roomId: number; message: Enriched.Message }>) => {
      const { roomId, message } = action.payload;

      const existing = state.messages[roomId] ?? [];
      const normalized = normalizeUserMessage(message);
      const next =
        existing.length >= MAX_ROOM_MESSAGES
          ? [...existing.slice(existing.length - MAX_ROOM_MESSAGES + 1), normalized]
          : [...existing, normalized];

      state.messages[roomId] = next;
    },

    updateGames: (state, action: PayloadAction<{ roomId: number; games: Data.ServerInfo_Game[] }>) => {
      const { roomId, games } = action.payload;
      const room = state.rooms[roomId];

      // An empty games array means no game updates — skip to avoid
      // accidentally wiping the existing normalized games map.
      if (!room || !games?.length) {
        return;
      }

      const gametypeMap = room.gametypeMap ?? {};

      for (const rawGame of games) {
        if (rawGame.closed) {
          delete room.games[rawGame.gameId];
          continue;
        }
        const existing = room.games[rawGame.gameId];
        if (existing) {
          // Merge the incoming proto into the existing snapshot.
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

    sortGames: (state, action: PayloadAction<{ field: App.GameSortField; order: App.SortDirection }>) => {
      // Sort is now derived in selectors; the reducer only stores the sort config.
      const { field, order } = action.payload;
      state.sortGamesBy = { field, order };
    },

    removeMessages: (state, action: PayloadAction<{ roomId: number; name: string; amount: number }>) => {
      const { name, amount, roomId } = action.payload;
      const roomMessages = state.messages[roomId];

      if (!roomMessages) {
        return;
      }

      // Drop the `amount` most-recent messages whose text starts with `${name}:`.
      // Walk newest → oldest so we remove the N latest matches.
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

    // Signal-only — no state mutation needed; explicit for discriminated-union exhaustiveness
    gameCreated: (_state, _action: PayloadAction<{ roomId: number }>) => {},
  },
});

export const roomsReducer = roomsSlice.reducer;
