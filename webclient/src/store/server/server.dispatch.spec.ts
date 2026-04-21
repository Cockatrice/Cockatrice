// @critical See rooms.dispatch.spec.ts — same hoisted-mockDispatch pattern.
const { mockDispatch } = vi.hoisted(() => ({ mockDispatch: vi.fn() }));
vi.mock('..', () => ({ store: { dispatch: mockDispatch } }));

import { Actions } from './server.actions';
import { Dispatch } from './server.dispatch';
import { Data } from '@app/types';
import { WebsocketTypes } from '@app/websocket/types';
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

beforeEach(() => {
  mockDispatch.mockClear();
});

describe('Dispatch', () => {
  it('initialized dispatches Actions.initialized()', () => {
    Dispatch.initialized();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.initialized());
  });

  it('clearStore dispatches Actions.clearStore()', () => {
    Dispatch.clearStore();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.clearStore());
  });

  it('connectionAttempted dispatches Actions.connectionAttempted()', () => {
    Dispatch.connectionAttempted();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.connectionAttempted());
  });

  it('loginSuccessful dispatches Actions.loginSuccessful()', () => {
    const options = makeLoginSuccessContext();
    Dispatch.loginSuccessful(options);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.loginSuccessful({ options }));
  });

  it('loginFailed dispatches Actions.loginFailed()', () => {
    Dispatch.loginFailed();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.loginFailed());
  });

  it('connectionFailed dispatches Actions.connectionFailed()', () => {
    Dispatch.connectionFailed();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.connectionFailed());
  });

  it('testConnectionSuccessful dispatches Actions.testConnectionSuccessful()', () => {
    Dispatch.testConnectionSuccessful(3);
    expect(mockDispatch).toHaveBeenCalledWith(
      Actions.testConnectionSuccessful({ serverOptions: 3 }),
    );
  });

  it('testConnectionFailed dispatches Actions.testConnectionFailed()', () => {
    Dispatch.testConnectionFailed();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.testConnectionFailed());
  });

  it('updateBuddyList dispatches Actions.updateBuddyList()', () => {
    const list = [makeUser()];
    Dispatch.updateBuddyList(list);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateBuddyList({ buddyList: list }));
  });

  it('addToBuddyList dispatches Actions.addToBuddyList()', () => {
    const user = makeUser();
    Dispatch.addToBuddyList(user);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.addToBuddyList({ user }));
  });

  it('removeFromBuddyList dispatches Actions.removeFromBuddyList()', () => {
    Dispatch.removeFromBuddyList('Alice');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.removeFromBuddyList({ userName: 'Alice' }));
  });

  it('updateIgnoreList dispatches Actions.updateIgnoreList()', () => {
    const list = [makeUser()];
    Dispatch.updateIgnoreList(list);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateIgnoreList({ ignoreList: list }));
  });

  it('addToIgnoreList dispatches Actions.addToIgnoreList()', () => {
    const user = makeUser();
    Dispatch.addToIgnoreList(user);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.addToIgnoreList({ user }));
  });

  it('removeFromIgnoreList dispatches Actions.removeFromIgnoreList()', () => {
    Dispatch.removeFromIgnoreList('Bob');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.removeFromIgnoreList({ userName: 'Bob' }));
  });

  it('updateInfo dispatches Actions.updateInfo({ info: { name, version } })', () => {
    Dispatch.updateInfo('Servatrice', '2.9');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateInfo({ info: { name: 'Servatrice', version: '2.9' } }));
  });

  it('updateStatus dispatches Actions.updateStatus({ status: { state, description } })', () => {
    Dispatch.updateStatus(WebsocketTypes.StatusEnum.CONNECTED, 'ok');
    expect(mockDispatch).toHaveBeenCalledWith(
      Actions.updateStatus({ status: { state: WebsocketTypes.StatusEnum.CONNECTED, description: 'ok' } })
    );
  });

  it('updateUser dispatches Actions.updateUser()', () => {
    const user = makeUser();
    Dispatch.updateUser(user);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateUser({ user }));
  });

  it('updateUsers dispatches Actions.updateUsers()', () => {
    const users = [makeUser()];
    Dispatch.updateUsers(users);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateUsers({ users }));
  });

  it('userJoined dispatches Actions.userJoined()', () => {
    const user = makeUser();
    Dispatch.userJoined(user);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.userJoined({ user }));
  });

  it('userLeft dispatches Actions.userLeft()', () => {
    Dispatch.userLeft('Carol');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.userLeft({ name: 'Carol' }));
  });

  it('viewLogs dispatches Actions.viewLogs()', () => {
    const logs = [create(Data.ServerInfo_ChatMessageSchema, { targetType: 'room' })];
    Dispatch.viewLogs(logs);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.viewLogs({ logs }));
  });

  it('clearLogs dispatches Actions.clearLogs()', () => {
    Dispatch.clearLogs();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.clearLogs());
  });

  it('serverMessage dispatches Actions.serverMessage()', () => {
    Dispatch.serverMessage('Welcome!');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.serverMessage({ message: 'Welcome!' }));
  });

  it('registrationRequiresEmail dispatches correctly', () => {
    Dispatch.registrationRequiresEmail();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.registrationRequiresEmail());
  });

  it('registrationSuccess dispatches correctly', () => {
    Dispatch.registrationSuccess();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.registrationSuccess());
  });

  it('registrationFailed passes reason and endTime to action', () => {
    Dispatch.registrationFailed('reason', 999);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.registrationFailed({ reason: 'reason', endTime: 999 }));
  });

  it('registrationFailed passes reason only when no endTime', () => {
    Dispatch.registrationFailed('plain reason');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.registrationFailed({ reason: 'plain reason', endTime: undefined }));
  });

  it('registrationEmailError dispatches correctly', () => {
    Dispatch.registrationEmailError('bad');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.registrationEmailError({ error: 'bad' }));
  });

  it('registrationPasswordError dispatches correctly', () => {
    Dispatch.registrationPasswordError('weak');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.registrationPasswordError({ error: 'weak' }));
  });

  it('registrationUserNameError dispatches correctly', () => {
    Dispatch.registrationUserNameError('taken');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.registrationUserNameError({ error: 'taken' }));
  });

  it('accountAwaitingActivation dispatches correctly', () => {
    const options = makePendingActivationContext();
    Dispatch.accountAwaitingActivation(options);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.accountAwaitingActivation({ options }));
  });

  it('accountActivationSuccess dispatches correctly', () => {
    Dispatch.accountActivationSuccess();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.accountActivationSuccess());
  });

  it('accountActivationFailed dispatches correctly', () => {
    Dispatch.accountActivationFailed();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.accountActivationFailed());
  });

  it('resetPassword dispatches correctly', () => {
    Dispatch.resetPassword();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.resetPassword());
  });

  it('resetPasswordFailed dispatches correctly', () => {
    Dispatch.resetPasswordFailed();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.resetPasswordFailed());
  });

  it('resetPasswordChallenge dispatches correctly', () => {
    Dispatch.resetPasswordChallenge();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.resetPasswordChallenge());
  });

  it('resetPasswordSuccess dispatches correctly', () => {
    Dispatch.resetPasswordSuccess();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.resetPasswordSuccess());
  });

  it('adjustMod dispatches Actions.adjustMod()', () => {
    Dispatch.adjustMod('Dan', true, false);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.adjustMod({ userName: 'Dan', shouldBeMod: true, shouldBeJudge: false }));
  });

  it('reloadConfig dispatches correctly', () => {
    Dispatch.reloadConfig();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.reloadConfig());
  });

  it('shutdownServer dispatches correctly', () => {
    Dispatch.shutdownServer();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.shutdownServer());
  });

  it('updateServerMessage dispatches correctly', () => {
    Dispatch.updateServerMessage();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateServerMessage());
  });

  it('accountPasswordChange dispatches correctly', () => {
    Dispatch.accountPasswordChange();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.accountPasswordChange());
  });

  it('accountEditChanged dispatches correctly', () => {
    const user = makeUser();
    Dispatch.accountEditChanged(user);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.accountEditChanged({ user }));
  });

  it('accountImageChanged dispatches correctly', () => {
    const user = makeUser();
    Dispatch.accountImageChanged(user);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.accountImageChanged({ user }));
  });

  it('getUserInfo dispatches correctly', () => {
    const userInfo = makeUser({ name: 'Frank' });
    Dispatch.getUserInfo(userInfo);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.getUserInfo({ userInfo }));
  });

  it('notifyUser dispatches correctly', () => {
    const notification = create(Data.Event_NotifyUserSchema, { type: 1, warningReason: '', customTitle: '', customContent: '' });
    Dispatch.notifyUser(notification);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.notifyUser({ notification }));
  });

  it('serverShutdown dispatches correctly', () => {
    const data = create(Data.Event_ServerShutdownSchema, { reason: 'maintenance', minutes: 5 });
    Dispatch.serverShutdown(data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.serverShutdown({ data }));
  });

  it('userMessage dispatches correctly', () => {
    const messageData = create(Data.Event_UserMessageSchema, { senderName: 'Alice', receiverName: 'Bob', message: 'hey' });
    Dispatch.userMessage(messageData);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.userMessage({ messageData }));
  });

  it('addToList dispatches correctly', () => {
    Dispatch.addToList('buddyList', 'Grace');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.addToList({ list: 'buddyList', userName: 'Grace' }));
  });

  it('removeFromList dispatches correctly', () => {
    Dispatch.removeFromList('buddyList', 'Hank');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.removeFromList({ list: 'buddyList', userName: 'Hank' }));
  });

  it('banFromServer dispatches correctly', () => {
    Dispatch.banFromServer('Ira');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.banFromServer({ userName: 'Ira' }));
  });

  it('banHistory dispatches correctly', () => {
    const history = [makeBanHistoryItem()];
    Dispatch.banHistory('Ira', history);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.banHistory({ userName: 'Ira', banHistory: history }));
  });

  it('warnHistory dispatches correctly', () => {
    const history = [makeWarnHistoryItem()];
    Dispatch.warnHistory('Jack', history);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.warnHistory({ userName: 'Jack', warnHistory: history }));
  });

  it('warnListOptions dispatches correctly', () => {
    const list = [makeWarnListItem()];
    Dispatch.warnListOptions(list);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.warnListOptions({ warnList: list }));
  });

  it('warnUser dispatches correctly', () => {
    Dispatch.warnUser('Kelly');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.warnUser({ userName: 'Kelly' }));
  });

  it('grantReplayAccess dispatches correctly', () => {
    Dispatch.grantReplayAccess(7, 'Moe');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.grantReplayAccess({ replayId: 7, moderatorName: 'Moe' }));
  });

  it('forceActivateUser dispatches correctly', () => {
    Dispatch.forceActivateUser('Ned', 'Moe');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.forceActivateUser({ usernameToActivate: 'Ned', moderatorName: 'Moe' }));
  });

  it('getAdminNotes dispatches correctly', () => {
    Dispatch.getAdminNotes('Ned', 'notes');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.getAdminNotes({ userName: 'Ned', notes: 'notes' }));
  });

  it('updateAdminNotes dispatches correctly', () => {
    Dispatch.updateAdminNotes('Ned', 'updated');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateAdminNotes({ userName: 'Ned', notes: 'updated' }));
  });

  it('replayList dispatches correctly', () => {
    const list = [makeReplayMatch()];
    Dispatch.replayList(list);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.replayList({ matchList: list }));
  });

  it('replayAdded dispatches correctly', () => {
    const match = makeReplayMatch();
    Dispatch.replayAdded(match);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.replayAdded({ matchInfo: match }));
  });

  it('replayModifyMatch dispatches correctly', () => {
    Dispatch.replayModifyMatch(5, true);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.replayModifyMatch({ gameId: 5, doNotHide: true }));
  });

  it('replayDeleteMatch dispatches correctly', () => {
    Dispatch.replayDeleteMatch(5);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.replayDeleteMatch({ gameId: 5 }));
  });

  it('backendDecks dispatches correctly', () => {
    const deckList = makeDeckList();
    Dispatch.backendDecks(deckList);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.backendDecks({ deckList }));
  });

  it('deckNewDir dispatches correctly', () => {
    Dispatch.deckNewDir('a/b', 'newFolder');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.deckNewDir({ path: 'a/b', dirName: 'newFolder' }));
  });

  it('deckDelDir dispatches correctly', () => {
    Dispatch.deckDelDir('a/b');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.deckDelDir({ path: 'a/b' }));
  });

  it('deckUpload dispatches correctly', () => {
    const treeItem = makeDeckTreeItem();
    Dispatch.deckUpload('a/b', treeItem);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.deckUpload({ path: 'a/b', treeItem }));
  });

  it('deckDelete dispatches correctly', () => {
    Dispatch.deckDelete(42);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.deckDelete({ deckId: 42 }));
  });

  it('deckDownloaded dispatches correctly', () => {
    Dispatch.deckDownloaded(42, '<deck-xml>');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.deckDownloaded({ deckId: 42, deck: '<deck-xml>' }));
  });

  it('replayDownloaded dispatches correctly', () => {
    const data = new Uint8Array([1, 2, 3]);
    Dispatch.replayDownloaded(99, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.replayDownloaded({ replayId: 99, replayData: data }));
  });

  it('gamesOfUser dispatches correctly', () => {
    const response = create(Data.Response_GetGamesOfUserSchema, { roomList: [], gameList: [] });
    Dispatch.gamesOfUser('alice', response);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.gamesOfUser({ userName: 'alice', response }));
  });

  it('clearRegistrationErrors dispatches correctly', () => {
    Dispatch.clearRegistrationErrors();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.clearRegistrationErrors());
  });
});
