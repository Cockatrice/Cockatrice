import { Actions } from './server.actions';
import { App, Data } from '@app/types';
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
    expect(Actions.initialized()).toEqual({ type: Types.INITIALIZED });
  });

  it('clearStore', () => {
    expect(Actions.clearStore()).toEqual({ type: Types.CLEAR_STORE });
  });

  it('connectionAttempted', () => {
    expect(Actions.connectionAttempted()).toEqual({ type: Types.CONNECTION_ATTEMPTED });
  });

  it('loginSuccessful', () => {
    const options = makeLoginSuccessContext();
    expect(Actions.loginSuccessful(options)).toEqual({ type: Types.LOGIN_SUCCESSFUL, options });
  });

  it('loginFailed', () => {
    expect(Actions.loginFailed()).toEqual({ type: Types.LOGIN_FAILED });
  });

  it('connectionFailed', () => {
    expect(Actions.connectionFailed()).toEqual({ type: Types.CONNECTION_FAILED });
  });

  it('testConnectionSuccessful', () => {
    expect(Actions.testConnectionSuccessful()).toEqual({ type: Types.TEST_CONNECTION_SUCCESSFUL });
  });

  it('testConnectionFailed', () => {
    expect(Actions.testConnectionFailed()).toEqual({ type: Types.TEST_CONNECTION_FAILED });
  });

  it('serverMessage', () => {
    expect(Actions.serverMessage('hello')).toEqual({ type: Types.SERVER_MESSAGE, message: 'hello' });
  });

  it('updateBuddyList', () => {
    const list = [makeUser()];
    expect(Actions.updateBuddyList(list)).toEqual({ type: Types.UPDATE_BUDDY_LIST, buddyList: list });
  });

  it('addToBuddyList', () => {
    const user = makeUser();
    expect(Actions.addToBuddyList(user)).toEqual({ type: Types.ADD_TO_BUDDY_LIST, user });
  });

  it('removeFromBuddyList', () => {
    expect(Actions.removeFromBuddyList('Alice')).toEqual({ type: Types.REMOVE_FROM_BUDDY_LIST, userName: 'Alice' });
  });

  it('updateIgnoreList', () => {
    const list = [makeUser()];
    expect(Actions.updateIgnoreList(list)).toEqual({ type: Types.UPDATE_IGNORE_LIST, ignoreList: list });
  });

  it('addToIgnoreList', () => {
    const user = makeUser();
    expect(Actions.addToIgnoreList(user)).toEqual({ type: Types.ADD_TO_IGNORE_LIST, user });
  });

  it('removeFromIgnoreList', () => {
    expect(Actions.removeFromIgnoreList('Bob')).toEqual({ type: Types.REMOVE_FROM_IGNORE_LIST, userName: 'Bob' });
  });

  it('updateInfo', () => {
    const info = { name: 'Servatrice', version: '2.0' };
    expect(Actions.updateInfo(info)).toEqual({ type: Types.UPDATE_INFO, info });
  });

  it('updateStatus', () => {
    const status = { state: App.StatusEnum.CONNECTED, description: 'connected' };
    expect(Actions.updateStatus(status)).toEqual({ type: Types.UPDATE_STATUS, status });
  });

  it('updateUser', () => {
    const user = makeUser();
    expect(Actions.updateUser(user)).toEqual({ type: Types.UPDATE_USER, user });
  });

  it('updateUsers', () => {
    const users = [makeUser()];
    expect(Actions.updateUsers(users)).toEqual({ type: Types.UPDATE_USERS, users });
  });

  it('userJoined', () => {
    const user = makeUser();
    expect(Actions.userJoined(user)).toEqual({ type: Types.USER_JOINED, user });
  });

  it('userLeft', () => {
    expect(Actions.userLeft('Carol')).toEqual({ type: Types.USER_LEFT, name: 'Carol' });
  });

  it('viewLogs', () => {
    const logs = [create(Data.ServerInfo_ChatMessageSchema, { targetType: 'room' })];
    expect(Actions.viewLogs(logs)).toEqual({ type: Types.VIEW_LOGS, logs });
  });

  it('clearLogs', () => {
    expect(Actions.clearLogs()).toEqual({ type: Types.CLEAR_LOGS });
  });

  it('registrationRequiresEmail', () => {
    expect(Actions.registrationRequiresEmail()).toEqual({ type: Types.REGISTRATION_REQUIRES_EMAIL });
  });

  it('registrationSuccess', () => {
    expect(Actions.registrationSuccess()).toEqual({ type: Types.REGISTRATION_SUCCESS });
  });

  it('registrationFailed', () => {
    expect(Actions.registrationFailed('err', 999)).toEqual({ type: Types.REGISTRATION_FAILED, reason: 'err', endTime: 999 });
  });

  it('registrationFailed without endTime', () => {
    expect(Actions.registrationFailed('err')).toEqual({ type: Types.REGISTRATION_FAILED, reason: 'err', endTime: undefined });
  });

  it('registrationEmailError', () => {
    expect(Actions.registrationEmailError('bad email')).toEqual({ type: Types.REGISTRATION_EMAIL_ERROR, error: 'bad email' });
  });

  it('registrationPasswordError', () => {
    expect(Actions.registrationPasswordError('bad pw')).toEqual({ type: Types.REGISTRATION_PASSWORD_ERROR, error: 'bad pw' });
  });

  it('registrationUserNameError', () => {
    expect(Actions.registrationUserNameError('bad name')).toEqual({ type: Types.REGISTRATION_USERNAME_ERROR, error: 'bad name' });
  });

  it('accountAwaitingActivation', () => {
    const options = makePendingActivationContext();
    expect(Actions.accountAwaitingActivation(options)).toEqual({ type: Types.ACCOUNT_AWAITING_ACTIVATION, options });
  });

  it('accountActivationSuccess', () => {
    expect(Actions.accountActivationSuccess()).toEqual({ type: Types.ACCOUNT_ACTIVATION_SUCCESS });
  });

  it('accountActivationFailed', () => {
    expect(Actions.accountActivationFailed()).toEqual({ type: Types.ACCOUNT_ACTIVATION_FAILED });
  });

  it('resetPassword', () => {
    expect(Actions.resetPassword()).toEqual({ type: Types.RESET_PASSWORD_REQUESTED });
  });

  it('resetPasswordFailed', () => {
    expect(Actions.resetPasswordFailed()).toEqual({ type: Types.RESET_PASSWORD_FAILED });
  });

  it('resetPasswordChallenge', () => {
    expect(Actions.resetPasswordChallenge()).toEqual({ type: Types.RESET_PASSWORD_CHALLENGE });
  });

  it('resetPasswordSuccess', () => {
    expect(Actions.resetPasswordSuccess()).toEqual({ type: Types.RESET_PASSWORD_SUCCESS });
  });

  it('adjustMod', () => {
    expect(Actions.adjustMod('Dan', true, false)).toEqual({
      type: Types.ADJUST_MOD,
      userName: 'Dan',
      shouldBeMod: true,
      shouldBeJudge: false,
    });
  });

  it('reloadConfig', () => {
    expect(Actions.reloadConfig()).toEqual({ type: Types.RELOAD_CONFIG });
  });

  it('shutdownServer', () => {
    expect(Actions.shutdownServer()).toEqual({ type: Types.SHUTDOWN_SERVER });
  });

  it('updateServerMessage', () => {
    expect(Actions.updateServerMessage()).toEqual({ type: Types.UPDATE_SERVER_MESSAGE });
  });

  it('accountPasswordChange', () => {
    expect(Actions.accountPasswordChange()).toEqual({ type: Types.ACCOUNT_PASSWORD_CHANGE });
  });

  it('accountEditChanged', () => {
    const user = makeUser();
    expect(Actions.accountEditChanged(user)).toEqual({ type: Types.ACCOUNT_EDIT_CHANGED, user });
  });

  it('accountImageChanged', () => {
    const user = makeUser();
    expect(Actions.accountImageChanged(user)).toEqual({ type: Types.ACCOUNT_IMAGE_CHANGED, user });
  });

  it('getUserInfo', () => {
    const userInfo = makeUser({ name: 'Frank' });
    expect(Actions.getUserInfo(userInfo)).toEqual({ type: Types.GET_USER_INFO, userInfo });
  });

  it('notifyUser', () => {
    const notification = create(Data.Event_NotifyUserSchema, { type: 1, warningReason: '', customTitle: '', customContent: '' });
    expect(Actions.notifyUser(notification)).toEqual({ type: Types.NOTIFY_USER, notification });
  });

  it('serverShutdown', () => {
    const data = create(Data.Event_ServerShutdownSchema, { reason: 'maintenance', minutes: 5 });
    expect(Actions.serverShutdown(data)).toEqual({ type: Types.SERVER_SHUTDOWN, data });
  });

  it('userMessage', () => {
    const messageData = create(Data.Event_UserMessageSchema, { senderName: 'Alice', receiverName: 'Bob', message: 'hey' });
    expect(Actions.userMessage(messageData)).toEqual({ type: Types.USER_MESSAGE, messageData });
  });

  it('addToList', () => {
    expect(Actions.addToList('buddyList', 'Grace')).toEqual({
      type: Types.ADD_TO_LIST,
      list: 'buddyList',
      userName: 'Grace',
    });
  });

  it('removeFromList', () => {
    expect(Actions.removeFromList('buddyList', 'Hank')).toEqual({
      type: Types.REMOVE_FROM_LIST,
      list: 'buddyList',
      userName: 'Hank',
    });
  });

  it('banFromServer', () => {
    expect(Actions.banFromServer('Ira')).toEqual({ type: Types.BAN_FROM_SERVER, userName: 'Ira' });
  });

  it('banHistory', () => {
    const history = [makeBanHistoryItem()];
    expect(Actions.banHistory('Ira', history)).toEqual({ type: Types.BAN_HISTORY, userName: 'Ira', banHistory: history });
  });

  it('warnHistory', () => {
    const history = [makeWarnHistoryItem()];
    expect(Actions.warnHistory('Jack', history)).toEqual({ type: Types.WARN_HISTORY, userName: 'Jack', warnHistory: history });
  });

  it('warnListOptions', () => {
    const list = [makeWarnListItem()];
    expect(Actions.warnListOptions(list)).toEqual({ type: Types.WARN_LIST_OPTIONS, warnList: list });
  });

  it('warnUser', () => {
    expect(Actions.warnUser('Kelly')).toEqual({ type: Types.WARN_USER, userName: 'Kelly' });
  });

  it('grantReplayAccess', () => {
    expect(Actions.grantReplayAccess(7, 'Moe')).toEqual({
      type: Types.GRANT_REPLAY_ACCESS,
      replayId: 7,
      moderatorName: 'Moe',
    });
  });

  it('forceActivateUser', () => {
    expect(Actions.forceActivateUser('Ned', 'Moe')).toEqual({
      type: Types.FORCE_ACTIVATE_USER,
      usernameToActivate: 'Ned',
      moderatorName: 'Moe',
    });
  });

  it('getAdminNotes', () => {
    expect(Actions.getAdminNotes('Ned', 'some notes')).toEqual({
      type: Types.GET_ADMIN_NOTES,
      userName: 'Ned',
      notes: 'some notes',
    });
  });

  it('updateAdminNotes', () => {
    expect(Actions.updateAdminNotes('Ned', 'updated notes')).toEqual({
      type: Types.UPDATE_ADMIN_NOTES,
      userName: 'Ned',
      notes: 'updated notes',
    });
  });

  it('replayList', () => {
    const list = [makeReplayMatch()];
    expect(Actions.replayList(list)).toEqual({ type: Types.REPLAY_LIST, matchList: list });
  });

  it('replayAdded', () => {
    const match = makeReplayMatch();
    expect(Actions.replayAdded(match)).toEqual({ type: Types.REPLAY_ADDED, matchInfo: match });
  });

  it('replayModifyMatch', () => {
    expect(Actions.replayModifyMatch(5, true)).toEqual({
      type: Types.REPLAY_MODIFY_MATCH,
      gameId: 5,
      doNotHide: true,
    });
  });

  it('replayDeleteMatch', () => {
    expect(Actions.replayDeleteMatch(5)).toEqual({ type: Types.REPLAY_DELETE_MATCH, gameId: 5 });
  });

  it('backendDecks', () => {
    const deckList = makeDeckList();
    expect(Actions.backendDecks(deckList)).toEqual({ type: Types.BACKEND_DECKS, deckList });
  });

  it('deckNewDir', () => {
    expect(Actions.deckNewDir('a/b', 'newFolder')).toEqual({
      type: Types.DECK_NEW_DIR,
      path: 'a/b',
      dirName: 'newFolder',
    });
  });

  it('deckDelDir', () => {
    expect(Actions.deckDelDir('a/b')).toEqual({ type: Types.DECK_DEL_DIR, path: 'a/b' });
  });

  it('deckUpload', () => {
    const treeItem = makeDeckTreeItem();
    expect(Actions.deckUpload('a/b', treeItem)).toEqual({
      type: Types.DECK_UPLOAD,
      path: 'a/b',
      treeItem,
    });
  });

  it('deckDelete', () => {
    expect(Actions.deckDelete(42)).toEqual({ type: Types.DECK_DELETE, deckId: 42 });
  });

  it('gamesOfUser', () => {
    const response = create(Data.Response_GetGamesOfUserSchema, { roomList: [], gameList: [] });
    expect(Actions.gamesOfUser('alice', response)).toEqual({ type: Types.GAMES_OF_USER, userName: 'alice', response });
  });

  it('clearRegistrationErrors', () => {
    expect(Actions.clearRegistrationErrors()).toEqual({ type: Types.CLEAR_REGISTRATION_ERRORS });
  });
});
