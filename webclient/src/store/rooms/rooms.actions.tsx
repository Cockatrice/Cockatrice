import { App, Data, Enriched } from '@app/types';

import { Types } from './rooms.types';

export const Actions = {
  clearStore: () => ({
    type: Types.CLEAR_STORE,
  }),

  updateRooms: (rooms: Data.ServerInfo_Room[]) => ({
    type: Types.UPDATE_ROOMS,
    rooms,
  }),

  joinRoom: (roomInfo: Data.ServerInfo_Room) => ({
    type: Types.JOIN_ROOM,
    roomInfo,
  }),

  leaveRoom: (roomId: number) => ({
    type: Types.LEAVE_ROOM,
    roomId,
  }),

  addMessage: (roomId: number, message: Enriched.Message) => ({
    type: Types.ADD_MESSAGE,
    roomId,
    message,
  }),

  updateGames: (roomId: number, games: Data.ServerInfo_Game[]) => ({
    type: Types.UPDATE_GAMES,
    roomId,
    games,
  }),

  userJoined: (roomId: number, user: Data.ServerInfo_User) => ({
    type: Types.USER_JOINED,
    roomId,
    user,
  }),

  userLeft: (roomId: number, name: string) => ({
    type: Types.USER_LEFT,
    roomId,
    name,
  }),

  sortGames: (roomId: number, field: App.GameSortField, order: App.SortDirection) => ({
    type: Types.SORT_GAMES,
    roomId,
    field,
    order,
  }),

  removeMessages: (roomId: number, name: string, amount: number) => ({
    type: Types.REMOVE_MESSAGES,
    roomId,
    name,
    amount,
  }),

  gameCreated: (roomId: number) => ({
    type: Types.GAME_CREATED,
    roomId,
  }),

  joinedGame: (roomId: number, gameId: number) => ({
    type: Types.JOINED_GAME,
    roomId,
    gameId,
  }),
}

export type RoomsAction = ReturnType<typeof Actions[keyof typeof Actions]>;
