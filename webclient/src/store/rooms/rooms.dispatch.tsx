import { Actions } from './rooms.actions';
import { store } from '../store';

export const Dispatch = {
  updateRooms: rooms => {
    store.dispatch(Actions.updateRooms(rooms));
  },

  joinRoom: roomInfo => {
    console.log(roomInfo);
    store.dispatch(Actions.joinRoom(roomInfo));
  },

  leaveRoom: roomId => {
    store.dispatch(Actions.leaveRoom(roomId));
  },

  addMessage: (roomId, message) => {
    store.dispatch(Actions.addMessage(roomId, message));
  },

  updateGames: (roomId, games) => {
    console.log('update', games);
    store.dispatch(Actions.updateGames(roomId, games));
  }
}