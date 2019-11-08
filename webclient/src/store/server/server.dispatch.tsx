import { Actions } from './server.actions';
import { store } from '../store';

export const Dispatch = {
  connectServer: () => {
    store.dispatch(Actions.connectServer());
  },
  connectionClosed: reason => {
    store.dispatch(Actions.connectionClosed(reason));
  },
  joinRoom: roomId => {
    store.dispatch(Actions.joinRoom(roomId));
  },
  updateInfo: (name, version) => {
    store.dispatch(Actions.updateInfo({
      name,
      version
    }));
  },
  updateRooms: rooms => {
    store.dispatch(Actions.updateRooms(rooms));
  },
  updateRoom: room => {
    store.dispatch(Actions.updateRoom(room));
  },
  updateStatus: (state, description) => {
    store.dispatch(Actions.updateStatus({
      state,
      description
    }));
  },
  updateUser: user => {
    store.dispatch(Actions.updateUser(user));
  },
  serverMessage: message => {
    store.dispatch(Actions.serverMessage(message));
  }
}