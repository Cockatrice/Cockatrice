import { GameSortField, Message, SortDirection, User } from 'types';
import type { ServerInfo_Room } from 'generated/proto/serverinfo_room_pb';
import type { ServerInfo_Game } from 'generated/proto/serverinfo_game_pb';

import { Types } from './rooms.types';

export const Actions = {
  clearStore: () => ({
    type: Types.CLEAR_STORE,
  }),

  updateRooms: (rooms: ServerInfo_Room[]) => ({
    type: Types.UPDATE_ROOMS,
    rooms,
  }),

  joinRoom: (roomInfo: ServerInfo_Room) => ({
    type: Types.JOIN_ROOM,
    roomInfo,
  }),

  leaveRoom: (roomId: number) => ({
    type: Types.LEAVE_ROOM,
    roomId,
  }),

  addMessage: (roomId: number, message: Message) => ({
    type: Types.ADD_MESSAGE,
    roomId,
    message,
  }),

  updateGames: (roomId: number, games: ServerInfo_Game[]) => ({
    type: Types.UPDATE_GAMES,
    roomId,
    games,
  }),

  userJoined: (roomId: number, user: User) => ({
    type: Types.USER_JOINED,
    roomId,
    user,
  }),

  userLeft: (roomId: number, name: string) => ({
    type: Types.USER_LEFT,
    roomId,
    name,
  }),

  sortGames: (roomId: number, field: GameSortField, order: SortDirection) => ({
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
