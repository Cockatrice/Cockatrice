import { App, Data, Enriched } from '@app/types';

import { Actions } from './rooms.actions';
import { GameFilters } from './rooms.interfaces';
import { store } from '..';

export const Dispatch = {
  clearStore: () => {
    store.dispatch(Actions.clearStore());
  },

  updateRooms: (rooms: Data.ServerInfo_Room[]) => {
    store.dispatch(Actions.updateRooms({ rooms }));
  },

  joinRoom: (roomInfo: Data.ServerInfo_Room) => {
    store.dispatch(Actions.joinRoom({ roomInfo }));
  },

  leaveRoom: (roomId: number) => {
    store.dispatch(Actions.leaveRoom({ roomId }));
  },

  addMessage: (roomId: number, message: Enriched.Message) => {
    store.dispatch(Actions.addMessage({ roomId, message }));
  },

  updateGames: (roomId: number, games: Data.ServerInfo_Game[]) => {
    store.dispatch(Actions.updateGames({ roomId, games }));
  },

  userJoined: (roomId: number, user: Data.ServerInfo_User) => {
    store.dispatch(Actions.userJoined({ roomId, user }));
  },

  userLeft: (roomId: number, name: string) => {
    store.dispatch(Actions.userLeft({ roomId, name }));
  },

  sortGames: (roomId: number, field: App.GameSortField, order: App.SortDirection) => {
    store.dispatch(Actions.sortGames({ roomId, field, order }));
  },

  removeMessages: (roomId: number, name: string, amount: number) => {
    store.dispatch(Actions.removeMessages({ roomId, name, amount }));
  },

  gameCreated: (roomId: number) => {
    store.dispatch(Actions.gameCreated({ roomId }));
  },

  joinedGame: (roomId: number, gameId: number) => {
    store.dispatch(Actions.joinedGame({ roomId, gameId }));
  },

  selectGame: (roomId: number, gameId: number | undefined) => {
    store.dispatch(Actions.selectGame({ roomId, gameId }));
  },

  setGameFilters: (roomId: number, filters: GameFilters) => {
    store.dispatch(Actions.setGameFilters({ roomId, filters }));
  },

  clearGameFilters: (roomId: number) => {
    store.dispatch(Actions.clearGameFilters({ roomId }));
  },
}
