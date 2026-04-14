import {
  GametypeMap, WebSocketConnectOptions
} from 'types';
import type { ServerInfo_User } from 'generated/proto/serverinfo_user_pb';
import type { ServerInfo_Ban } from 'generated/proto/serverinfo_ban_pb';
import type { ServerInfo_Warning } from 'generated/proto/serverinfo_warning_pb';
import type { Response_WarnList } from 'generated/proto/response_warn_list_pb';
import type { ServerInfo_ReplayMatch } from 'generated/proto/serverinfo_replay_match_pb';
import type { Response_DeckList } from 'generated/proto/response_deck_list_pb';
import type { ServerInfo_DeckStorage_TreeItem } from 'generated/proto/serverinfo_deckstorage_pb';
import type { ServerInfo_ChatMessage } from 'generated/proto/serverinfo_chat_message_pb';
import type { ServerInfo_Game } from 'generated/proto/serverinfo_game_pb';
import { NotifyUserData, ServerShutdownData, UserMessageData } from 'websocket/events/session/interfaces';
import { ServerStateStatus } from './server.interfaces';
import { Types } from './server.types';

export const Actions = {
  initialized: () => ({
    type: Types.INITIALIZED
  }),
  clearStore: () => ({
    type: Types.CLEAR_STORE
  }),
  connectionAttempted: () => ({
    type: Types.CONNECTION_ATTEMPTED
  }),
  loginSuccessful: (options: WebSocketConnectOptions) => ({
    type: Types.LOGIN_SUCCESSFUL,
    options
  }),
  loginFailed: () => ({
    type: Types.LOGIN_FAILED,
  }),
  connectionClosed: (reason: number) => ({
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
  serverMessage: (message: string) => ({
    type: Types.SERVER_MESSAGE,
    message
  }),
  updateBuddyList: (buddyList: ServerInfo_User[]) => ({
    type: Types.UPDATE_BUDDY_LIST,
    buddyList
  }),
  addToBuddyList: (user: ServerInfo_User) => ({
    type: Types.ADD_TO_BUDDY_LIST,
    user
  }),
  removeFromBuddyList: (userName: string) => ({
    type: Types.REMOVE_FROM_BUDDY_LIST,
    userName
  }),
  updateIgnoreList: (ignoreList: ServerInfo_User[]) => ({
    type: Types.UPDATE_IGNORE_LIST,
    ignoreList
  }),
  addToIgnoreList: (user: ServerInfo_User) => ({
    type: Types.ADD_TO_IGNORE_LIST,
    user
  }),
  removeFromIgnoreList: (userName: string) => ({
    type: Types.REMOVE_FROM_IGNORE_LIST,
    userName
  }),
  updateInfo: (info: { name: string; version: string }) => ({
    type: Types.UPDATE_INFO,
    info
  }),
  updateStatus: (status: ServerStateStatus) => ({
    type: Types.UPDATE_STATUS,
    status
  }),
  updateUser: (user: ServerInfo_User) => ({
    type: Types.UPDATE_USER,
    user
  }),
  updateUsers: (users: ServerInfo_User[]) => ({
    type: Types.UPDATE_USERS,
    users
  }),
  userJoined: (user: ServerInfo_User) => ({
    type: Types.USER_JOINED,
    user
  }),
  userLeft: (name: string) => ({
    type: Types.USER_LEFT,
    name
  }),
  viewLogs: (logs: ServerInfo_ChatMessage[]) => ({
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
    type: Types.REGISTRATION_SUCCESS,
  }),
  registrationFailed: (reason: string, endTime?: number) => ({
    type: Types.REGISTRATION_FAILED,
    reason,
    endTime,
  }),
  registrationEmailError: (error: string) => ({
    type: Types.REGISTRATION_EMAIL_ERROR,
    error
  }),
  registrationPasswordError: (error: string) => ({
    type: Types.REGISTRATION_PASSWORD_ERROR,
    error
  }),
  registrationUserNameError: (error: string) => ({
    type: Types.REGISTRATION_USERNAME_ERROR,
    error
  }),
  clearRegistrationErrors: () => ({
    type: Types.CLEAR_REGISTRATION_ERRORS,
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
  adjustMod: (userName: string, shouldBeMod: boolean, shouldBeJudge: boolean) => ({
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
  accountEditChanged: (user: Partial<ServerInfo_User>) => ({
    type: Types.ACCOUNT_EDIT_CHANGED,
    user,
  }),
  accountImageChanged: (user: Partial<ServerInfo_User>) => ({
    type: Types.ACCOUNT_IMAGE_CHANGED,
    user,
  }),
  getUserInfo: (userInfo: ServerInfo_User) => ({
    type: Types.GET_USER_INFO,
    userInfo,
  }),
  notifyUser: (notification: NotifyUserData) => ({
    type: Types.NOTIFY_USER,
    notification,
  }),
  serverShutdown: (data: ServerShutdownData) => ({
    type: Types.SERVER_SHUTDOWN,
    data,
  }),
  userMessage: (messageData: UserMessageData) => ({
    type: Types.USER_MESSAGE,
    messageData,
  }),
  addToList: (list: string, userName: string) => ({
    type: Types.ADD_TO_LIST,
    list,
    userName,
  }),
  removeFromList: (list: string, userName: string) => ({
    type: Types.REMOVE_FROM_LIST,
    list,
    userName,
  }),
  banFromServer: (userName: string) => ({
    type: Types.BAN_FROM_SERVER,
    userName,
  }),
  banHistory: (userName: string, banHistory: ServerInfo_Ban[]) => ({
    type: Types.BAN_HISTORY,
    userName,
    banHistory,
  }),
  warnHistory: (userName: string, warnHistory: ServerInfo_Warning[]) => ({
    type: Types.WARN_HISTORY,
    userName,
    warnHistory,
  }),
  warnListOptions: (warnList: Response_WarnList[]) => ({
    type: Types.WARN_LIST_OPTIONS,
    warnList,
  }),
  warnUser: (userName: string) => ({
    type: Types.WARN_USER,
    userName,
  }),
  grantReplayAccess: (replayId: number, moderatorName: string) => ({
    type: Types.GRANT_REPLAY_ACCESS,
    replayId,
    moderatorName,
  }),
  forceActivateUser: (usernameToActivate: string, moderatorName: string) => ({
    type: Types.FORCE_ACTIVATE_USER,
    usernameToActivate,
    moderatorName,
  }),
  getAdminNotes: (userName: string, notes: string) => ({
    type: Types.GET_ADMIN_NOTES,
    userName,
    notes,
  }),
  updateAdminNotes: (userName: string, notes: string) => ({
    type: Types.UPDATE_ADMIN_NOTES,
    userName,
    notes,
  }),
  replayList: (matchList: ServerInfo_ReplayMatch[]) => ({ type: Types.REPLAY_LIST, matchList }),
  replayAdded: (matchInfo: ServerInfo_ReplayMatch) => ({ type: Types.REPLAY_ADDED, matchInfo }),
  replayModifyMatch: (gameId: number, doNotHide: boolean) => ({ type: Types.REPLAY_MODIFY_MATCH, gameId, doNotHide }),
  replayDeleteMatch: (gameId: number) => ({ type: Types.REPLAY_DELETE_MATCH, gameId }),
  backendDecks: (deckList: Response_DeckList) => ({ type: Types.BACKEND_DECKS, deckList }),
  deckNewDir: (path: string, dirName: string) => ({ type: Types.DECK_NEW_DIR, path, dirName }),
  deckDelDir: (path: string) => ({ type: Types.DECK_DEL_DIR, path }),
  deckUpload: (path: string, treeItem: ServerInfo_DeckStorage_TreeItem) => ({ type: Types.DECK_UPLOAD, path, treeItem }),
  deckDelete: (deckId: number) => ({ type: Types.DECK_DELETE, deckId }),
  gamesOfUser: (userName: string, games: ServerInfo_Game[], gametypeMap: GametypeMap) =>
    ({ type: Types.GAMES_OF_USER, userName, games, gametypeMap }),
}

export type ServerAction = ReturnType<typeof Actions[keyof typeof Actions]>;
