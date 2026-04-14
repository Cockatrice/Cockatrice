import { Actions } from './server.actions';
import { store } from 'store';
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

export const Dispatch = {
  initialized: () => {
    store.dispatch(Actions.initialized());
  },
  clearStore: () => {
    store.dispatch(Actions.clearStore());
  },
  connectionAttempted: () => {
    store.dispatch(Actions.connectionAttempted());
  },
  loginSuccessful: (options: WebSocketConnectOptions) => {
    store.dispatch(Actions.loginSuccessful(options));
  },
  loginFailed: () => {
    store.dispatch(Actions.loginFailed());
  },
  connectionClosed: (reason: number) => {
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
  updateBuddyList: (buddyList: ServerInfo_User[]) => {
    store.dispatch(Actions.updateBuddyList(buddyList));
  },
  addToBuddyList: (user: ServerInfo_User) => {
    store.dispatch(Actions.addToBuddyList(user));
  },
  removeFromBuddyList: (userName: string) => {
    store.dispatch(Actions.removeFromBuddyList(userName));
  },
  updateIgnoreList: (ignoreList: ServerInfo_User[]) => {
    store.dispatch(Actions.updateIgnoreList(ignoreList));
  },
  addToIgnoreList: (user: ServerInfo_User) => {
    store.dispatch(Actions.addToIgnoreList(user));
  },
  removeFromIgnoreList: (userName: string) => {
    store.dispatch(Actions.removeFromIgnoreList(userName));
  },
  updateInfo: (name: string, version: string) => {
    store.dispatch(Actions.updateInfo({
      name,
      version
    }));
  },
  updateStatus: (state: number, description: string) => {
    store.dispatch(Actions.updateStatus({
      state,
      description
    }));
  },
  updateUser: (user: ServerInfo_User) => {
    store.dispatch(Actions.updateUser(user));
  },
  updateUsers: (users: ServerInfo_User[]) => {
    store.dispatch(Actions.updateUsers(users));
  },
  userJoined: (user: ServerInfo_User) => {
    store.dispatch(Actions.userJoined(user));
  },
  userLeft: (name: string) => {
    store.dispatch(Actions.userLeft(name));
  },
  viewLogs: (logs: ServerInfo_ChatMessage[]) => {
    store.dispatch(Actions.viewLogs(logs));
  },
  clearLogs: () => {
    store.dispatch(Actions.clearLogs());
  },
  serverMessage: (message: string) => {
    store.dispatch(Actions.serverMessage(message));
  },
  registrationRequiresEmail: () => {
    store.dispatch(Actions.registrationRequiresEmail());
  },
  registrationSuccess: () => {
    store.dispatch(Actions.registrationSuccess())
  },
  registrationFailed: (reason: string, endTime?: number) => {
    store.dispatch(Actions.registrationFailed(reason, endTime));
  },
  clearRegistrationErrors: () => {
    store.dispatch(Actions.clearRegistrationErrors());
  },
  registrationEmailError: (error: string) => {
    store.dispatch(Actions.registrationEmailError(error));
  },
  registrationPasswordError: (error: string) => {
    store.dispatch(Actions.registrationPasswordError(error));
  },
  registrationUserNameError: (error: string) => {
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
  adjustMod: (userName: string, shouldBeMod: boolean, shouldBeJudge: boolean) => {
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
  accountEditChanged: (user: Partial<ServerInfo_User>) => {
    store.dispatch(Actions.accountEditChanged(user));
  },
  accountImageChanged: (user: Partial<ServerInfo_User>) => {
    store.dispatch(Actions.accountImageChanged(user));
  },
  getUserInfo: (userInfo: ServerInfo_User) => {
    store.dispatch(Actions.getUserInfo(userInfo));
  },
  notifyUser: (notification: NotifyUserData) => {
    store.dispatch(Actions.notifyUser(notification))
  },
  serverShutdown: (data: ServerShutdownData) => {
    store.dispatch(Actions.serverShutdown(data))
  },
  userMessage: (messageData: UserMessageData) => {
    store.dispatch(Actions.userMessage(messageData))
  },
  addToList: (list: string, userName: string) => {
    store.dispatch(Actions.addToList(list, userName))
  },
  removeFromList: (list: string, userName: string) => {
    store.dispatch(Actions.removeFromList(list, userName))
  },
  banFromServer: (userName: string) => {
    store.dispatch(Actions.banFromServer(userName));
  },
  banHistory: (userName: string, banHistory: ServerInfo_Ban[]) => {
    store.dispatch(Actions.banHistory(userName, banHistory))
  },
  warnHistory: (userName: string, warnHistory: ServerInfo_Warning[]) => {
    store.dispatch(Actions.warnHistory(userName, warnHistory))
  },
  warnListOptions: (warnList: Response_WarnList[]) => {
    store.dispatch(Actions.warnListOptions(warnList))
  },
  warnUser: (userName: string) => {
    store.dispatch(Actions.warnUser(userName))
  },
  grantReplayAccess: (replayId: number, moderatorName: string) => {
    store.dispatch(Actions.grantReplayAccess(replayId, moderatorName));
  },
  forceActivateUser: (usernameToActivate: string, moderatorName: string) => {
    store.dispatch(Actions.forceActivateUser(usernameToActivate, moderatorName));
  },
  getAdminNotes: (userName: string, notes: string) => {
    store.dispatch(Actions.getAdminNotes(userName, notes));
  },
  updateAdminNotes: (userName: string, notes: string) => {
    store.dispatch(Actions.updateAdminNotes(userName, notes));
  },
  replayList: (matchList: ServerInfo_ReplayMatch[]) => {
    store.dispatch(Actions.replayList(matchList));
  },
  replayAdded: (matchInfo: ServerInfo_ReplayMatch) => {
    store.dispatch(Actions.replayAdded(matchInfo));
  },
  replayModifyMatch: (gameId: number, doNotHide: boolean) => {
    store.dispatch(Actions.replayModifyMatch(gameId, doNotHide));
  },
  replayDeleteMatch: (gameId: number) => {
    store.dispatch(Actions.replayDeleteMatch(gameId));
  },
  backendDecks: (deckList: Response_DeckList) => {
    store.dispatch(Actions.backendDecks(deckList));
  },
  deckNewDir: (path: string, dirName: string) => {
    store.dispatch(Actions.deckNewDir(path, dirName));
  },
  deckDelDir: (path: string) => {
    store.dispatch(Actions.deckDelDir(path));
  },
  deckUpload: (path: string, treeItem: ServerInfo_DeckStorage_TreeItem) => {
    store.dispatch(Actions.deckUpload(path, treeItem));
  },
  deckDelete: (deckId: number) => {
    store.dispatch(Actions.deckDelete(deckId));
  },
  gamesOfUser: (userName: string, games: ServerInfo_Game[], gametypeMap: GametypeMap) => {
    store.dispatch(Actions.gamesOfUser(userName, games, gametypeMap));
  },
}
