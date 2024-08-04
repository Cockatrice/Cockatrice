import { reset } from 'redux-form';
import { Actions } from './server.actions';
import { store } from 'store';
import { WebSocketConnectOptions } from 'types';

export const Dispatch = {
  initialized: () => {
    store.dispatch(Actions.initialized());
  },
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
  connectionFailed: () => {
    store.dispatch(Actions.connectionFailed());
  },
  testConnectionSuccessful: () => {
    store.dispatch(Actions.testConnectionSuccessful());
  },
  testConnectionFailed: () => {
    store.dispatch(Actions.testConnectionFailed());
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
  registrationSuccess: () => {
    store.dispatch(Actions.registrationSuccess())
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
  accountAwaitingActivation: (options: WebSocketConnectOptions) => {
    store.dispatch(Actions.accountAwaitingActivation(options));
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
  },
  adjustMod: (userName, shouldBeMod, shouldBeJudge) => {
    store.dispatch(Actions.adjustMod(userName, shouldBeMod, shouldBeJudge));
  },
  reloadConfig: () => {
    store.dispatch(Actions.reloadConfig());
  },
  shutdownServer: () => {
    store.dispatch(Actions.shutdownServer());
  },
  updateServerMessage: () => {
    store.dispatch(Actions.updateServerMessage());
  },
  accountPasswordChange: () => {
    store.dispatch(Actions.accountPasswordChange());
  },
  accountEditChanged: (user) => {
    store.dispatch(Actions.accountEditChanged(user));
  },
  accountImageChanged: (user) => {
    store.dispatch(Actions.accountImageChanged(user));
  },
  directMessageSent: (userName, message) => {
    store.dispatch(Actions.directMessageSent(userName, message));
  },
  getUserInfo: (userInfo) => {
    store.dispatch(Actions.getUserInfo(userInfo));
  },
  notifyUser: (notification) => {
    store.dispatch(Actions.notifyUser(notification))
  },
  serverShutdown: (data) => {
    store.dispatch(Actions.serverShutdown(data))
  },
  userMessage: (messageData) => {
    store.dispatch(Actions.userMessage(messageData))
  },
  addToList: (list, userName) => {
    store.dispatch(Actions.addToList(list, userName))
  },
  removeFromList: (list, userName) => {
    store.dispatch(Actions.removeFromList(list, userName))
  },
  banFromServer: (userName) => {
    store.dispatch(Actions.banFromServer(userName));
  },
  banHistory: (userName, banHistory) => {
    store.dispatch(Actions.banHistory(userName, banHistory))
  },
  warnHistory: (userName, warnHistory) => {
    store.dispatch(Actions.warnHistory(userName, warnHistory))
  },
  warnListOptions: (warnList) => {
    store.dispatch(Actions.warnListOptions(warnList))
  },
  warnUser: (userName) => {
    store.dispatch(Actions.warnUser(userName))
  },
}
