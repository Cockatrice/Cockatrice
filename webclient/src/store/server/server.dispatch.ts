import { Actions } from './server.actions';
import { store } from '..';
import { Data, Enriched } from '@app/types';

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
  loginSuccessful: (options: Enriched.LoginSuccessContext) => {
    store.dispatch(Actions.loginSuccessful({ options }));
  },
  loginFailed: () => {
    store.dispatch(Actions.loginFailed());
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
  updateBuddyList: (buddyList: Data.ServerInfo_User[]) => {
    store.dispatch(Actions.updateBuddyList({ buddyList }));
  },
  addToBuddyList: (user: Data.ServerInfo_User) => {
    store.dispatch(Actions.addToBuddyList({ user }));
  },
  removeFromBuddyList: (userName: string) => {
    store.dispatch(Actions.removeFromBuddyList({ userName }));
  },
  updateIgnoreList: (ignoreList: Data.ServerInfo_User[]) => {
    store.dispatch(Actions.updateIgnoreList({ ignoreList }));
  },
  addToIgnoreList: (user: Data.ServerInfo_User) => {
    store.dispatch(Actions.addToIgnoreList({ user }));
  },
  removeFromIgnoreList: (userName: string) => {
    store.dispatch(Actions.removeFromIgnoreList({ userName }));
  },
  updateInfo: (name: string, version: string) => {
    store.dispatch(Actions.updateInfo({ info: { name, version } }));
  },
  updateStatus: (state: Enriched.StatusEnum, description: string) => {
    store.dispatch(Actions.updateStatus({ status: { state, description } }));
  },
  updateUser: (user: Data.ServerInfo_User) => {
    store.dispatch(Actions.updateUser({ user }));
  },
  updateUsers: (users: Data.ServerInfo_User[]) => {
    store.dispatch(Actions.updateUsers({ users }));
  },
  userJoined: (user: Data.ServerInfo_User) => {
    store.dispatch(Actions.userJoined({ user }));
  },
  userLeft: (name: string) => {
    store.dispatch(Actions.userLeft({ name }));
  },
  viewLogs: (logs: Data.ServerInfo_ChatMessage[]) => {
    store.dispatch(Actions.viewLogs({ logs }));
  },
  clearLogs: () => {
    store.dispatch(Actions.clearLogs());
  },
  serverMessage: (message: string) => {
    store.dispatch(Actions.serverMessage({ message }));
  },
  registrationRequiresEmail: () => {
    store.dispatch(Actions.registrationRequiresEmail());
  },
  registrationSuccess: () => {
    store.dispatch(Actions.registrationSuccess());
  },
  registrationFailed: (reason: string, endTime?: number) => {
    store.dispatch(Actions.registrationFailed({ reason, endTime }));
  },
  clearRegistrationErrors: () => {
    store.dispatch(Actions.clearRegistrationErrors());
  },
  registrationEmailError: (error: string) => {
    store.dispatch(Actions.registrationEmailError({ error }));
  },
  registrationPasswordError: (error: string) => {
    store.dispatch(Actions.registrationPasswordError({ error }));
  },
  registrationUserNameError: (error: string) => {
    store.dispatch(Actions.registrationUserNameError({ error }));
  },
  accountAwaitingActivation: (options: Enriched.PendingActivationContext) => {
    store.dispatch(Actions.accountAwaitingActivation({ options }));
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
    store.dispatch(Actions.adjustMod({ userName, shouldBeMod, shouldBeJudge }));
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
  accountEditChanged: (user: Partial<Data.ServerInfo_User>) => {
    store.dispatch(Actions.accountEditChanged({ user }));
  },
  accountImageChanged: (user: Partial<Data.ServerInfo_User>) => {
    store.dispatch(Actions.accountImageChanged({ user }));
  },
  getUserInfo: (userInfo: Data.ServerInfo_User) => {
    store.dispatch(Actions.getUserInfo({ userInfo }));
  },
  notifyUser: (notification: Data.Event_NotifyUser) => {
    store.dispatch(Actions.notifyUser({ notification }));
  },
  serverShutdown: (data: Data.Event_ServerShutdown) => {
    store.dispatch(Actions.serverShutdown({ data }));
  },
  userMessage: (messageData: Data.Event_UserMessage) => {
    store.dispatch(Actions.userMessage({ messageData }));
  },
  addToList: (list: string, userName: string) => {
    store.dispatch(Actions.addToList({ list, userName }));
  },
  removeFromList: (list: string, userName: string) => {
    store.dispatch(Actions.removeFromList({ list, userName }));
  },
  banFromServer: (userName: string) => {
    store.dispatch(Actions.banFromServer({ userName }));
  },
  banHistory: (userName: string, banHistory: Data.ServerInfo_Ban[]) => {
    store.dispatch(Actions.banHistory({ userName, banHistory }));
  },
  warnHistory: (userName: string, warnHistory: Data.ServerInfo_Warning[]) => {
    store.dispatch(Actions.warnHistory({ userName, warnHistory }));
  },
  warnListOptions: (warnList: Data.Response_WarnList[]) => {
    store.dispatch(Actions.warnListOptions({ warnList }));
  },
  warnUser: (userName: string) => {
    store.dispatch(Actions.warnUser({ userName }));
  },
  grantReplayAccess: (replayId: number, moderatorName: string) => {
    store.dispatch(Actions.grantReplayAccess({ replayId, moderatorName }));
  },
  forceActivateUser: (usernameToActivate: string, moderatorName: string) => {
    store.dispatch(Actions.forceActivateUser({ usernameToActivate, moderatorName }));
  },
  getAdminNotes: (userName: string, notes: string) => {
    store.dispatch(Actions.getAdminNotes({ userName, notes }));
  },
  updateAdminNotes: (userName: string, notes: string) => {
    store.dispatch(Actions.updateAdminNotes({ userName, notes }));
  },
  replayList: (matchList: Data.ServerInfo_ReplayMatch[]) => {
    store.dispatch(Actions.replayList({ matchList }));
  },
  replayAdded: (matchInfo: Data.ServerInfo_ReplayMatch) => {
    store.dispatch(Actions.replayAdded({ matchInfo }));
  },
  replayModifyMatch: (gameId: number, doNotHide: boolean) => {
    store.dispatch(Actions.replayModifyMatch({ gameId, doNotHide }));
  },
  replayDeleteMatch: (gameId: number) => {
    store.dispatch(Actions.replayDeleteMatch({ gameId }));
  },
  backendDecks: (deckList: Data.Response_DeckList) => {
    store.dispatch(Actions.backendDecks({ deckList }));
  },
  deckNewDir: (path: string, dirName: string) => {
    store.dispatch(Actions.deckNewDir({ path, dirName }));
  },
  deckDelDir: (path: string) => {
    store.dispatch(Actions.deckDelDir({ path }));
  },
  deckUpload: (path: string, treeItem: Data.ServerInfo_DeckStorage_TreeItem) => {
    store.dispatch(Actions.deckUpload({ path, treeItem }));
  },
  deckDelete: (deckId: number) => {
    store.dispatch(Actions.deckDelete({ deckId }));
  },
  deckDownloaded: (deckId: number, deck: string) => {
    store.dispatch(Actions.deckDownloaded({ deckId, deck }));
  },
  replayDownloaded: (replayId: number, replayData: Uint8Array) => {
    store.dispatch(Actions.replayDownloaded({ replayId, replayData }));
  },
  gamesOfUser: (userName: string, response: Data.Response_GetGamesOfUser) => {
    store.dispatch(Actions.gamesOfUser({ userName, response }));
  },
};
