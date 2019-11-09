import { Actions } from './rooms.actions';
import { store } from '../store';

export const Dispatch = {
  updateRooms: rooms => {
    store.dispatch(Actions.updateRooms(rooms));
  },

  joinRoom: roomId => {
    store.dispatch(Actions.joinRoom(roomId));
  },

  leaveRoom: roomId => {
    store.dispatch(Actions.leaveRoom(roomId));
  },

  addMessage: (roomId, message) => {
    store.dispatch(Actions.addMessage(roomId, message));
  }
}