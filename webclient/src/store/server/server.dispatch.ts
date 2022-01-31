import { reset } from 'redux-form';
import { Actions } from './server.actions';
import { store } from 'store';

export const Dispatch = {
  clearStore: () => {
    store.dispatch(Actions.clearStore());
  },
  loginSuccessful: options => {
    store.dispatch(Actions.loginSuccessful(options));
  },
  loginFailed: () => {
    store.dispatch(Actions.loginFailed());
  },
  connectionClosed: reason => {
    store.dispatch(Actions.connectionClosed(reason));
  },
  updateBuddyList: buddyList => {
    store.dispatch(Actions.updateBuddyList(buddyList));
  },
  addToBuddyList: user => {
    store.dispatch(reset('addToBuddies'));
    store.dispatch(Actions.addToBuddyList(user));
  },
  removeFromBuddyList: userName => {
    store.dispatch(Actions.removeFromBuddyList(userName));
  },
  updateIgnoreList: ignoreList => {
    store.dispatch(Actions.updateIgnoreList(ignoreList));
  },
  addToIgnoreList: user => {
    store.dispatch(reset('addToIgnore'));
    store.dispatch(Actions.addToIgnoreList(user));
  },
  removeFromIgnoreList: userName => {
    store.dispatch(Actions.removeFromIgnoreList(userName));
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
  userJoined: user => {
    store.dispatch(Actions.userJoined(user));
  },
  userLeft: name => {
    store.dispatch(Actions.userLeft(name));
  },
  viewLogs: name => {
    store.dispatch(Actions.viewLogs(name));
  },
  clearLogs: () => {
    store.dispatch(Actions.clearLogs());
  },
  serverMessage: message => {
    store.dispatch(Actions.serverMessage(message));
  },
  registrationRequiresEmail: () => {
    store.dispatch(Actions.registrationRequiresEmail());
  },
  registrationFailed: (error) => {
    store.dispatch(Actions.registrationFailed(error));
  },
  registrationEmailError: (error) => {
    store.dispatch(Actions.registrationEmailError(error));
  },
  registrationPasswordError: (error) => {
    store.dispatch(Actions.registrationPasswordError(error));
  },
  registrationUserNameError: (error) => {
    store.dispatch(Actions.registrationUserNameError(error));
  },
  accountAwaitingActivation: () => {
    store.dispatch(Actions.accountAwaitingActivation());
  },
  accountActivationSuccess: () => {
    store.dispatch(Actions.accountActivationSuccess());
  },
  accountActivationFailed: () => {
    store.dispatch(Actions.accountActivationFailed());
  },
  resetPassword: () => {
    store.dispatch(Actions.resetPassword());
  },
  resetPasswordFailed: () => {
    store.dispatch(Actions.resetPasswordFailed());
  },
  resetPasswordChallenge: () => {
    store.dispatch(Actions.resetPasswordChallenge());
  },
  resetPasswordSuccess: () => {
    store.dispatch(Actions.resetPasswordSuccess());
  }
}
