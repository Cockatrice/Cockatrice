import { Actions } from './server.actions';
import { Data } from '@app/types';
import { WebsocketTypes } from '@app/websocket/types';
import { Types } from './server.types';
import { create } from '@bufbuild/protobuf';
import {
  makeBanHistoryItem,
  makeLoginSuccessContext,
  makePendingActivationContext,
  makeDeckList,
  makeDeckTreeItem,
  makeReplayMatch,
  makeUser,
  makeWarnHistoryItem,
  makeWarnListItem,
} from './__mocks__/server-fixtures';

describe('Actions', () => {
  it('initialized', () => {
    expect(Actions.initialized()).toEqual({ type: Types.INITIALIZED, payload: undefined });
  });

  it('clearStore', () => {
    expect(Actions.clearStore()).toEqual({ type: Types.CLEAR_STORE, payload: undefined });
  });

  it('connectionAttempted', () => {
    expect(Actions.connectionAttempted()).toEqual({ type: Types.CONNECTION_ATTEMPTED, payload: undefined });
  });

  it('loginSuccessful', () => {
    const options = makeLoginSuccessContext();
    expect(Actions.loginSuccessful({ options })).toEqual({ type: Types.LOGIN_SUCCESSFUL, payload: { options } });
  });

  it('loginFailed', () => {
    expect(Actions.loginFailed()).toEqual({ type: Types.LOGIN_FAILED, payload: undefined });
  });

  it('connectionFailed', () => {
    expect(Actions.connectionFailed()).toEqual({ type: Types.CONNECTION_FAILED, payload: undefined });
  });

  it('testConnectionSuccessful', () => {
    expect(Actions.testConnectionSuccessful()).toEqual({ type: Types.TEST_CONNECTION_SUCCESSFUL, payload: undefined });
  });

  it('testConnectionFailed', () => {
    expect(Actions.testConnectionFailed()).toEqual({ type: Types.TEST_CONNECTION_FAILED, payload: undefined });
  });

  it('serverMessage', () => {
    expect(Actions.serverMessage({ message: 'hello' })).toEqual({ type: Types.SERVER_MESSAGE, payload: { message: 'hello' } });
  });

  it('updateBuddyList', () => {
    const list = [makeUser()];
    expect(Actions.updateBuddyList({ buddyList: list })).toEqual({ type: Types.UPDATE_BUDDY_LIST, payload: { buddyList: list } });
  });

  it('addToBuddyList', () => {
    const user = makeUser();
    expect(Actions.addToBuddyList({ user })).toEqual({ type: Types.ADD_TO_BUDDY_LIST, payload: { user } });
  });

  it('removeFromBuddyList', () => {
    const action = Actions.removeFromBuddyList({ userName: 'Alice' });
    expect(action).toEqual({ type: Types.REMOVE_FROM_BUDDY_LIST, payload: { userName: 'Alice' } });
  });

  it('updateIgnoreList', () => {
    const list = [makeUser()];
    expect(Actions.updateIgnoreList({ ignoreList: list })).toEqual({ type: Types.UPDATE_IGNORE_LIST, payload: { ignoreList: list } });
  });

  it('addToIgnoreList', () => {
    const user = makeUser();
    expect(Actions.addToIgnoreList({ user })).toEqual({ type: Types.ADD_TO_IGNORE_LIST, payload: { user } });
  });

  it('removeFromIgnoreList', () => {
    const action = Actions.removeFromIgnoreList({ userName: 'Bob' });
    expect(action).toEqual({ type: Types.REMOVE_FROM_IGNORE_LIST, payload: { userName: 'Bob' } });
  });

  it('updateInfo', () => {
    const info = { name: 'Servatrice', version: '2.0' };
    expect(Actions.updateInfo({ info })).toEqual({ type: Types.UPDATE_INFO, payload: { info } });
  });

  it('updateStatus', () => {
    const status = { state: WebsocketTypes.StatusEnum.CONNECTED, description: 'connected' };
    expect(Actions.updateStatus({ status })).toEqual({ type: Types.UPDATE_STATUS, payload: { status } });
  });

  it('updateUser', () => {
    const user = makeUser();
    expect(Actions.updateUser({ user })).toEqual({ type: Types.UPDATE_USER, payload: { user } });
  });

  it('updateUsers', () => {
    const users = [makeUser()];
    expect(Actions.updateUsers({ users })).toEqual({ type: Types.UPDATE_USERS, payload: { users } });
  });

  it('userJoined', () => {
    const user = makeUser();
    expect(Actions.userJoined({ user })).toEqual({ type: Types.USER_JOINED, payload: { user } });
  });

  it('userLeft', () => {
    expect(Actions.userLeft({ name: 'Carol' })).toEqual({ type: Types.USER_LEFT, payload: { name: 'Carol' } });
  });

  it('viewLogs', () => {
    const logs = [create(Data.ServerInfo_ChatMessageSchema, { targetType: 'room' })];
    expect(Actions.viewLogs({ logs })).toEqual({ type: Types.VIEW_LOGS, payload: { logs } });
  });

  it('clearLogs', () => {
    expect(Actions.clearLogs()).toEqual({ type: Types.CLEAR_LOGS, payload: undefined });
  });

  it('registrationRequiresEmail', () => {
    expect(Actions.registrationRequiresEmail()).toEqual({ type: Types.REGISTRATION_REQUIRES_EMAIL, payload: undefined });
  });

  it('registrationSuccess', () => {
    expect(Actions.registrationSuccess()).toEqual({ type: Types.REGISTRATION_SUCCESS, payload: undefined });
  });

  it('registrationFailed', () => {
    const action = Actions.registrationFailed({ reason: 'err', endTime: 999 });
    expect(action.payload).toEqual({ reason: 'err', endTime: 999 });
  });

  it('registrationFailed without endTime', () => {
    const action = Actions.registrationFailed({ reason: 'err' });
    expect(action.payload).toEqual({ reason: 'err' });
  });

  it('registrationEmailError', () => {
    const action = Actions.registrationEmailError({ error: 'bad email' });
    expect(action.payload).toEqual({ error: 'bad email' });
  });

  it('registrationPasswordError', () => {
    const action = Actions.registrationPasswordError({ error: 'bad pw' });
    expect(action.payload).toEqual({ error: 'bad pw' });
  });

  it('registrationUserNameError', () => {
    const action = Actions.registrationUserNameError({ error: 'bad name' });
    expect(action.payload).toEqual({ error: 'bad name' });
  });

  it('accountAwaitingActivation', () => {
    const options = makePendingActivationContext();
    expect(Actions.accountAwaitingActivation({ options })).toEqual({ type: Types.ACCOUNT_AWAITING_ACTIVATION, payload: { options } });
  });

  it('accountActivationSuccess', () => {
    expect(Actions.accountActivationSuccess()).toEqual({ type: Types.ACCOUNT_ACTIVATION_SUCCESS, payload: undefined });
  });

  it('accountActivationFailed', () => {
    expect(Actions.accountActivationFailed()).toEqual({ type: Types.ACCOUNT_ACTIVATION_FAILED, payload: undefined });
  });

  it('resetPassword', () => {
    expect(Actions.resetPassword()).toEqual({ type: Types.RESET_PASSWORD_REQUESTED, payload: undefined });
  });

  it('resetPasswordFailed', () => {
    expect(Actions.resetPasswordFailed()).toEqual({ type: Types.RESET_PASSWORD_FAILED, payload: undefined });
  });

  it('resetPasswordChallenge', () => {
    expect(Actions.resetPasswordChallenge()).toEqual({ type: Types.RESET_PASSWORD_CHALLENGE, payload: undefined });
  });

  it('resetPasswordSuccess', () => {
    expect(Actions.resetPasswordSuccess()).toEqual({ type: Types.RESET_PASSWORD_SUCCESS, payload: undefined });
  });

  it('adjustMod', () => {
    expect(Actions.adjustMod({ userName: 'Dan', shouldBeMod: true, shouldBeJudge: false })).toEqual({
      type: Types.ADJUST_MOD,
      payload: { userName: 'Dan', shouldBeMod: true, shouldBeJudge: false },
    });
  });

  it('reloadConfig', () => {
    expect(Actions.reloadConfig()).toEqual({ type: Types.RELOAD_CONFIG, payload: undefined });
  });

  it('shutdownServer', () => {
    expect(Actions.shutdownServer()).toEqual({ type: Types.SHUTDOWN_SERVER, payload: undefined });
  });

  it('updateServerMessage', () => {
    expect(Actions.updateServerMessage()).toEqual({ type: Types.UPDATE_SERVER_MESSAGE, payload: undefined });
  });

  it('accountPasswordChange', () => {
    expect(Actions.accountPasswordChange()).toEqual({ type: Types.ACCOUNT_PASSWORD_CHANGE, payload: undefined });
  });

  it('accountEditChanged', () => {
    const user = makeUser();
    expect(Actions.accountEditChanged({ user })).toEqual({ type: Types.ACCOUNT_EDIT_CHANGED, payload: { user } });
  });

  it('accountImageChanged', () => {
    const user = makeUser();
    expect(Actions.accountImageChanged({ user })).toEqual({ type: Types.ACCOUNT_IMAGE_CHANGED, payload: { user } });
  });

  it('getUserInfo', () => {
    const userInfo = makeUser({ name: 'Frank' });
    expect(Actions.getUserInfo({ userInfo })).toEqual({ type: Types.GET_USER_INFO, payload: { userInfo } });
  });

  it('notifyUser', () => {
    const notification = create(Data.Event_NotifyUserSchema, { type: 1, warningReason: '', customTitle: '', customContent: '' });
    expect(Actions.notifyUser({ notification })).toEqual({ type: Types.NOTIFY_USER, payload: { notification } });
  });

  it('serverShutdown', () => {
    const data = create(Data.Event_ServerShutdownSchema, { reason: 'maintenance', minutes: 5 });
    expect(Actions.serverShutdown({ data })).toEqual({ type: Types.SERVER_SHUTDOWN, payload: { data } });
  });

  it('userMessage', () => {
    const messageData = create(Data.Event_UserMessageSchema, { senderName: 'Alice', receiverName: 'Bob', message: 'hey' });
    expect(Actions.userMessage({ messageData })).toEqual({ type: Types.USER_MESSAGE, payload: { messageData } });
  });

  it('addToList', () => {
    expect(Actions.addToList({ list: 'buddyList', userName: 'Grace' })).toEqual({
      type: Types.ADD_TO_LIST,
      payload: { list: 'buddyList', userName: 'Grace' },
    });
  });

  it('removeFromList', () => {
    expect(Actions.removeFromList({ list: 'buddyList', userName: 'Hank' })).toEqual({
      type: Types.REMOVE_FROM_LIST,
      payload: { list: 'buddyList', userName: 'Hank' },
    });
  });

  it('banFromServer', () => {
    expect(Actions.banFromServer({ userName: 'Ira' })).toEqual({ type: Types.BAN_FROM_SERVER, payload: { userName: 'Ira' } });
  });

  it('banHistory', () => {
    const history = [makeBanHistoryItem()];
    const action = Actions.banHistory({ userName: 'Ira', banHistory: history });
    expect(action.payload).toEqual({ userName: 'Ira', banHistory: history });
  });

  it('warnHistory', () => {
    const history = [makeWarnHistoryItem()];
    const action = Actions.warnHistory({ userName: 'Jack', warnHistory: history });
    expect(action.payload).toEqual({ userName: 'Jack', warnHistory: history });
  });

  it('warnListOptions', () => {
    const list = [makeWarnListItem()];
    expect(Actions.warnListOptions({ warnList: list })).toEqual({ type: Types.WARN_LIST_OPTIONS, payload: { warnList: list } });
  });

  it('warnUser', () => {
    expect(Actions.warnUser({ userName: 'Kelly' })).toEqual({ type: Types.WARN_USER, payload: { userName: 'Kelly' } });
  });

  it('grantReplayAccess', () => {
    expect(Actions.grantReplayAccess({ replayId: 7, moderatorName: 'Moe' })).toEqual({
      type: Types.GRANT_REPLAY_ACCESS,
      payload: { replayId: 7, moderatorName: 'Moe' },
    });
  });

  it('forceActivateUser', () => {
    expect(Actions.forceActivateUser({ usernameToActivate: 'Ned', moderatorName: 'Moe' })).toEqual({
      type: Types.FORCE_ACTIVATE_USER,
      payload: { usernameToActivate: 'Ned', moderatorName: 'Moe' },
    });
  });

  it('getAdminNotes', () => {
    expect(Actions.getAdminNotes({ userName: 'Ned', notes: 'some notes' })).toEqual({
      type: Types.GET_ADMIN_NOTES,
      payload: { userName: 'Ned', notes: 'some notes' },
    });
  });

  it('updateAdminNotes', () => {
    expect(Actions.updateAdminNotes({ userName: 'Ned', notes: 'updated notes' })).toEqual({
      type: Types.UPDATE_ADMIN_NOTES,
      payload: { userName: 'Ned', notes: 'updated notes' },
    });
  });

  it('replayList', () => {
    const list = [makeReplayMatch()];
    expect(Actions.replayList({ matchList: list })).toEqual({ type: Types.REPLAY_LIST, payload: { matchList: list } });
  });

  it('replayAdded', () => {
    const match = makeReplayMatch();
    expect(Actions.replayAdded({ matchInfo: match })).toEqual({ type: Types.REPLAY_ADDED, payload: { matchInfo: match } });
  });

  it('replayModifyMatch', () => {
    expect(Actions.replayModifyMatch({ gameId: 5, doNotHide: true })).toEqual({
      type: Types.REPLAY_MODIFY_MATCH,
      payload: { gameId: 5, doNotHide: true },
    });
  });

  it('replayDeleteMatch', () => {
    expect(Actions.replayDeleteMatch({ gameId: 5 })).toEqual({ type: Types.REPLAY_DELETE_MATCH, payload: { gameId: 5 } });
  });

  it('backendDecks', () => {
    const deckList = makeDeckList();
    expect(Actions.backendDecks({ deckList })).toEqual({ type: Types.BACKEND_DECKS, payload: { deckList } });
  });

  it('deckNewDir', () => {
    expect(Actions.deckNewDir({ path: 'a/b', dirName: 'newFolder' })).toEqual({
      type: Types.DECK_NEW_DIR,
      payload: { path: 'a/b', dirName: 'newFolder' },
    });
  });

  it('deckDelDir', () => {
    expect(Actions.deckDelDir({ path: 'a/b' })).toEqual({ type: Types.DECK_DEL_DIR, payload: { path: 'a/b' } });
  });

  it('deckUpload', () => {
    const treeItem = makeDeckTreeItem();
    expect(Actions.deckUpload({ path: 'a/b', treeItem })).toEqual({
      type: Types.DECK_UPLOAD,
      payload: { path: 'a/b', treeItem },
    });
  });

  it('deckDelete', () => {
    expect(Actions.deckDelete({ deckId: 42 })).toEqual({ type: Types.DECK_DELETE, payload: { deckId: 42 } });
  });

  it('deckDownloaded', () => {
    expect(Actions.deckDownloaded({ deckId: 42, deck: '<xml>' })).toEqual({
      type: Types.DECK_DOWNLOADED,
      payload: { deckId: 42, deck: '<xml>' },
    });
  });

  it('replayDownloaded', () => {
    const replayData = new Uint8Array([1, 2, 3]);
    expect(Actions.replayDownloaded({ replayId: 99, replayData })).toEqual({
      type: Types.REPLAY_DOWNLOADED,
      payload: { replayId: 99, replayData },
    });
  });

  it('gamesOfUser', () => {
    const response = create(Data.Response_GetGamesOfUserSchema, { roomList: [], gameList: [] });
    const action = Actions.gamesOfUser({ userName: 'alice', response });
    expect(action.payload).toEqual({ userName: 'alice', response });
  });

  it('clearRegistrationErrors', () => {
    expect(Actions.clearRegistrationErrors()).toEqual({ type: Types.CLEAR_REGISTRATION_ERRORS, payload: undefined });
  });
});
