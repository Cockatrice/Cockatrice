import { WebSocketConnectOptions } from 'types';
import { Types } from './server.types';

export const Actions = {
  initialized: () => ({
    type: Types.INITIALIZED
  }),
  clearStore: () => ({
    type: Types.CLEAR_STORE
  }),
  loginSuccessful: (options: WebSocketConnectOptions) => ({
    type: Types.LOGIN_SUCCESSFUL,
    options
  }),
  loginFailed: () => ({
    type: Types.LOGIN_FAILED,
  }),
  connectionClosed: reason => ({
    type: Types.CONNECTION_CLOSED,
    reason
  }),
  connectionFailed: () => ({
    type: Types.CONNECTION_FAILED,
  }),
  testConnectionSuccessful: () => ({
    type: Types.TEST_CONNECTION_SUCCESSFUL,
  }),
  testConnectionFailed: () => ({
    type: Types.TEST_CONNECTION_FAILED,
  }),
  serverMessage: message => ({
    type: Types.SERVER_MESSAGE,
    message
  }),
  updateBuddyList: buddyList => ({
    type: Types.UPDATE_BUDDY_LIST,
    buddyList
  }),
  addToBuddyList: user => ({
    type: Types.ADD_TO_BUDDY_LIST,
    user
  }),
  removeFromBuddyList: userName => ({
    type: Types.REMOVE_FROM_BUDDY_LIST,
    userName
  }),
  updateIgnoreList: ignoreList => ({
    type: Types.UPDATE_IGNORE_LIST,
    ignoreList
  }),
  addToIgnoreList: user => ({
    type: Types.ADD_TO_IGNORE_LIST,
    user
  }),
  removeFromIgnoreList: userName => ({
    type: Types.REMOVE_FROM_IGNORE_LIST,
    userName
  }),
  updateInfo: info => ({
    type: Types.UPDATE_INFO,
    info
  }),
  updateStatus: status => ({
    type: Types.UPDATE_STATUS,
    status
  }),
  updateUser: user => ({
    type: Types.UPDATE_USER,
    user
  }),
  updateUsers: users => ({
    type: Types.UPDATE_USERS,
    users
  }),
  userJoined: user => ({
    type: Types.USER_JOINED,
    user
  }),
  userLeft: name => ({
    type: Types.USER_LEFT,
    name
  }),
  viewLogs: logs => ({
    type: Types.VIEW_LOGS,
    logs
  }),
  clearLogs: () => ({
    type: Types.CLEAR_LOGS,
  }),
  registrationRequiresEmail: () => ({
    type: Types.REGISTRATION_REQUIRES_EMAIL,
  }),
  registrationSuccess: () => ({
    type: Types.REGISTRATION_SUCCES,
  }),
  registrationFailed: (error) => ({
    type: Types.REGISTRATION_FAILED,
    error
  }),
  registrationEmailError: (error) => ({
    type: Types.REGISTRATION_EMAIL_ERROR,
    error
  }),
  registrationPasswordError: (error) => ({
    type: Types.REGISTRATION_PASSWORD_ERROR,
    error
  }),
  registrationUserNameError: (error) => ({
    type: Types.REGISTRATION_USERNAME_ERROR,
    error
  }),
  accountAwaitingActivation: (options: WebSocketConnectOptions) => ({
    type: Types.ACCOUNT_AWAITING_ACTIVATION,
    options
  }),
  accountActivationSuccess: () => ({
    type: Types.ACCOUNT_ACTIVATION_SUCCESS,
  }),
  accountActivationFailed: () => ({
    type: Types.ACCOUNT_ACTIVATION_FAILED,
  }),
  resetPassword: () => ({
    type: Types.RESET_PASSWORD_REQUESTED,
  }),
  resetPasswordFailed: () => ({
    type: Types.RESET_PASSWORD_FAILED,
  }),
  resetPasswordChallenge: () => ({
    type: Types.RESET_PASSWORD_CHALLENGE,
  }),
  resetPasswordSuccess: () => ({
    type: Types.RESET_PASSWORD_SUCCESS,
  }),
  adjustMod: (userName, shouldBeMod, shouldBeJudge) => ({
    type: Types.ADJUST_MOD,
    userName,
    shouldBeMod,
    shouldBeJudge,
  }),
  reloadConfig: () => ({
    type: Types.RELOAD_CONFIG,
  }),
  shutdownServer: () => ({
    type: Types.SHUTDOWN_SERVER,
  }),
  updateServerMessage: () => ({
    type: Types.UPDATE_SERVER_MESSAGE,
  }),
  accountPasswordChange: () => ({
    type: Types.ACCOUNT_PASSWORD_CHANGE,
  }),
  accountEditChanged: (user) => ({
    type: Types.ACCOUNT_EDIT_CHANGED,
    user,
  }),
  accountImageChanged: (user) => ({
    type: Types.ACCOUNT_IMAGE_CHANGED,
    user,
  }),
  directMessageSent: (userName, message) => ({
    type: Types.DIRECT_MESSAGE_SENT,
    userName,
    message,
  }),
  getUserInfo: (userInfo) => ({
    type: Types.GET_USER_INFO,
    userInfo,
  }),
  notifyUser: (notification) => ({
    type: Types.NOTIFY_USER,
    notification,
  }),
  serverShutdown: (data) => ({
    type: Types.SERVER_SHUTDOWN,
    data,
  }),
  userMessage: (messageData) => ({
    type: Types.USER_MESSAGE,
    messageData,
  }),
  addToList: (list, userName) => ({
    type: Types.ADD_TO_LIST,
    list,
    userName,
  }),
  removeFromList: (list, userName) => ({
    type: Types.REMOVE_FROM_LIST,
    list,
    userName,
  }),
  banFromServer: (userName) => ({
    type: Types.BAN_FROM_SERVER,
    userName,
  }),
  banHistory: (userName, banHistory) => ({
    type: Types.BAN_HISTORY,
    userName,
    banHistory,
  }),
  warnHistory: (userName, warnHistory) => ({
    type: Types.WARN_HISTORY,
    userName,
    warnHistory,
  }),
  warnListOptions: (warnList) => ({
    type: Types.WARN_LIST_OPTIONS,
    warnList,
  }),
  warnUser: (userName) => ({
    type: Types.WARN_USER,
    userName,
  }),
}
