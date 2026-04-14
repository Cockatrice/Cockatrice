import { GameSortField, Message, SortDirection, User } from 'types';
import type { ServerInfo_Room } from 'generated/proto/serverinfo_room_pb';
import type { ServerInfo_Game } from 'generated/proto/serverinfo_game_pb';

import { Actions } from './rooms.actions';
import { store } from 'store';

export const Dispatch = {
  clearStore: () => {
    store.dispatch(Actions.clearStore());
  },

  updateRooms: (rooms: ServerInfo_Room[]) => {
    store.dispatch(Actions.updateRooms(rooms));
  },

  joinRoom: (roomInfo: ServerInfo_Room) => {
    store.dispatch(Actions.joinRoom(roomInfo));

  },

  leaveRoom: (roomId: number) => {
    store.dispatch(Actions.leaveRoom(roomId));
  },

  addMessage: (roomId: number, message: Message) => {
    store.dispatch(Actions.addMessage(roomId, message));
  },

  updateGames: (roomId: number, games: ServerInfo_Game[]) => {
    store.dispatch(Actions.updateGames(roomId, games));
  },

  userJoined: (roomId: number, user: User) => {
    store.dispatch(Actions.userJoined(roomId, user));
  },

  userLeft: (roomId: number, name: string) => {
    store.dispatch(Actions.userLeft(roomId, name));
  },

  sortGames: (roomId: number, field: GameSortField, order: SortDirection) => {
    store.dispatch(Actions.sortGames(roomId, field, order));
  },

  removeMessages: (roomId: number, name: string, amount: number) => {
    store.dispatch(Actions.removeMessages(roomId, name, amount));
  },

  gameCreated: (roomId: number) => {
    store.dispatch(Actions.gameCreated(roomId));
  },

  joinedGame: (roomId: number, gameId: number) => {
    store.dispatch(Actions.joinedGame(roomId, gameId));
  }
}
