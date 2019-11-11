import { Actions } from './server.actions';
import { store } from '../store';

export const Dispatch = {
  connectServer: () => {
    store.dispatch(Actions.connectServer());
  },
  connectionClosed: reason => {
    store.dispatch(Actions.connectionClosed(reason));
  },
  updateInfo: (name, version) => {
    store.dispatch(Actions.updateInfo({
      name,
      version
    }));
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
  updateUsers: users => {
    store.dispatch(Actions.updateUsers(users));
  },
  serverMessage: message => {
    store.dispatch(Actions.serverMessage(message));
  }
}