vi.mock('store', () => ({ store: { dispatch: vi.fn() } }));

import { store } from 'store';
import { Actions } from './server.actions';
import { Dispatch } from './server.dispatch';
import { create } from '@bufbuild/protobuf';
import { Event_NotifyUserSchema } from 'generated/proto/event_notify_user_pb';
import { Event_ServerShutdownSchema } from 'generated/proto/event_server_shutdown_pb';
import { Event_UserMessageSchema } from 'generated/proto/event_user_message_pb';
import {
  makeBanHistoryItem,
  makeConnectOptions,
  makeDeckList,
  makeDeckTreeItem,
  makeGame,
  makeReplayMatch,
  makeUser,
  makeWarnHistoryItem,
  makeWarnListItem,
} from './__mocks__/server-fixtures';

beforeEach(() => vi.clearAllMocks());

describe('Dispatch', () => {
  it('initialized dispatches Actions.initialized()', () => {
    Dispatch.initialized();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.initialized());
  });

  it('clearStore dispatches Actions.clearStore()', () => {
    Dispatch.clearStore();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.clearStore());
  });

  it('loginSuccessful dispatches Actions.loginSuccessful()', () => {
    const options = makeConnectOptions();
    Dispatch.loginSuccessful(options);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.loginSuccessful(options));
  });

  it('loginFailed dispatches Actions.loginFailed()', () => {
    Dispatch.loginFailed();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.loginFailed());
  });

  it('connectionClosed dispatches Actions.connectionClosed()', () => {
    Dispatch.connectionClosed(3);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.connectionClosed(3));
  });

  it('connectionFailed dispatches Actions.connectionFailed()', () => {
    Dispatch.connectionFailed();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.connectionFailed());
  });

  it('testConnectionSuccessful dispatches Actions.testConnectionSuccessful()', () => {
    Dispatch.testConnectionSuccessful();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.testConnectionSuccessful());
  });

  it('testConnectionFailed dispatches Actions.testConnectionFailed()', () => {
    Dispatch.testConnectionFailed();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.testConnectionFailed());
  });

  it('updateBuddyList dispatches Actions.updateBuddyList()', () => {
    const list = [makeUser()];
    Dispatch.updateBuddyList(list);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateBuddyList(list));
  });

  it('addToBuddyList dispatches Actions.addToBuddyList()', () => {
    const user = makeUser();
    Dispatch.addToBuddyList(user);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.addToBuddyList(user));
  });

  it('removeFromBuddyList dispatches Actions.removeFromBuddyList()', () => {
    Dispatch.removeFromBuddyList('Alice');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.removeFromBuddyList('Alice'));
  });

  it('updateIgnoreList dispatches Actions.updateIgnoreList()', () => {
    const list = [makeUser()];
    Dispatch.updateIgnoreList(list);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateIgnoreList(list));
  });

  it('addToIgnoreList dispatches Actions.addToIgnoreList()', () => {
    const user = makeUser();
    Dispatch.addToIgnoreList(user);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.addToIgnoreList(user));
  });

  it('removeFromIgnoreList dispatches Actions.removeFromIgnoreList()', () => {
    Dispatch.removeFromIgnoreList('Bob');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.removeFromIgnoreList('Bob'));
  });

  it('updateInfo dispatches Actions.updateInfo({ name, version })', () => {
    Dispatch.updateInfo('Servatrice', '2.9');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateInfo({ name: 'Servatrice', version: '2.9' }));
  });

  it('updateStatus dispatches Actions.updateStatus({ state, description })', () => {
    Dispatch.updateStatus(2, 'ok');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateStatus({ state: 2, description: 'ok' }));
  });

  it('updateUser dispatches Actions.updateUser()', () => {
    const user = makeUser();
    Dispatch.updateUser(user);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateUser(user));
  });

  it('updateUsers dispatches Actions.updateUsers()', () => {
    const users = [makeUser()];
    Dispatch.updateUsers(users);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateUsers(users));
  });

  it('userJoined dispatches Actions.userJoined()', () => {
    const user = makeUser();
    Dispatch.userJoined(user);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.userJoined(user));
  });

  it('userLeft dispatches Actions.userLeft()', () => {
    Dispatch.userLeft('Carol');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.userLeft('Carol'));
  });

  it('viewLogs dispatches Actions.viewLogs()', () => {
    const logs = [{ targetType: 'room' }] as any[];
    Dispatch.viewLogs(logs);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.viewLogs(logs));
  });

  it('clearLogs dispatches Actions.clearLogs()', () => {
    Dispatch.clearLogs();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.clearLogs());
  });

  it('serverMessage dispatches Actions.serverMessage()', () => {
    Dispatch.serverMessage('Welcome!');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.serverMessage('Welcome!'));
  });

  it('registrationRequiresEmail dispatches correctly', () => {
    Dispatch.registrationRequiresEmail();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.registrationRequiresEmail());
  });

  it('registrationSuccess dispatches correctly', () => {
    Dispatch.registrationSuccess();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.registrationSuccess());
  });

  it('registrationFailed passes reason and endTime to action', () => {
    Dispatch.registrationFailed('reason', 999);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.registrationFailed('reason', 999));
  });

  it('registrationFailed passes reason only when no endTime', () => {
    Dispatch.registrationFailed('plain reason');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.registrationFailed('plain reason', undefined));
  });

  it('registrationEmailError dispatches correctly', () => {
    Dispatch.registrationEmailError('bad');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.registrationEmailError('bad'));
  });

  it('registrationPasswordError dispatches correctly', () => {
    Dispatch.registrationPasswordError('weak');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.registrationPasswordError('weak'));
  });

  it('registrationUserNameError dispatches correctly', () => {
    Dispatch.registrationUserNameError('taken');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.registrationUserNameError('taken'));
  });

  it('accountAwaitingActivation dispatches correctly', () => {
    const options = makeConnectOptions();
    Dispatch.accountAwaitingActivation(options);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.accountAwaitingActivation(options));
  });

  it('accountActivationSuccess dispatches correctly', () => {
    Dispatch.accountActivationSuccess();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.accountActivationSuccess());
  });

  it('accountActivationFailed dispatches correctly', () => {
    Dispatch.accountActivationFailed();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.accountActivationFailed());
  });

  it('resetPassword dispatches correctly', () => {
    Dispatch.resetPassword();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.resetPassword());
  });

  it('resetPasswordFailed dispatches correctly', () => {
    Dispatch.resetPasswordFailed();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.resetPasswordFailed());
  });

  it('resetPasswordChallenge dispatches correctly', () => {
    Dispatch.resetPasswordChallenge();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.resetPasswordChallenge());
  });

  it('resetPasswordSuccess dispatches correctly', () => {
    Dispatch.resetPasswordSuccess();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.resetPasswordSuccess());
  });

  it('adjustMod dispatches Actions.adjustMod()', () => {
    Dispatch.adjustMod('Dan', true, false);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.adjustMod('Dan', true, false));
  });

  it('reloadConfig dispatches correctly', () => {
    Dispatch.reloadConfig();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.reloadConfig());
  });

  it('shutdownServer dispatches correctly', () => {
    Dispatch.shutdownServer();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.shutdownServer());
  });

  it('updateServerMessage dispatches correctly', () => {
    Dispatch.updateServerMessage();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateServerMessage());
  });

  it('accountPasswordChange dispatches correctly', () => {
    Dispatch.accountPasswordChange();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.accountPasswordChange());
  });

  it('accountEditChanged dispatches correctly', () => {
    const user = makeUser();
    Dispatch.accountEditChanged(user);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.accountEditChanged(user));
  });

  it('accountImageChanged dispatches correctly', () => {
    const user = makeUser();
    Dispatch.accountImageChanged(user);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.accountImageChanged(user));
  });

  it('getUserInfo dispatches correctly', () => {
    const userInfo = makeUser({ name: 'Frank' });
    Dispatch.getUserInfo(userInfo);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.getUserInfo(userInfo));
  });

  it('notifyUser dispatches correctly', () => {
    const notification = create(Event_NotifyUserSchema, { type: 1, warningReason: '', customTitle: '', customContent: '' });
    Dispatch.notifyUser(notification);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.notifyUser(notification));
  });

  it('serverShutdown dispatches correctly', () => {
    const data = create(Event_ServerShutdownSchema, { reason: 'maintenance', minutes: 5 });
    Dispatch.serverShutdown(data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.serverShutdown(data));
  });

  it('userMessage dispatches correctly', () => {
    const messageData = create(Event_UserMessageSchema, { senderName: 'Alice', receiverName: 'Bob', message: 'hey' });
    Dispatch.userMessage(messageData);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.userMessage(messageData));
  });

  it('addToList dispatches correctly', () => {
    Dispatch.addToList('buddyList', 'Grace');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.addToList('buddyList', 'Grace'));
  });

  it('removeFromList dispatches correctly', () => {
    Dispatch.removeFromList('buddyList', 'Hank');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.removeFromList('buddyList', 'Hank'));
  });

  it('banFromServer dispatches correctly', () => {
    Dispatch.banFromServer('Ira');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.banFromServer('Ira'));
  });

  it('banHistory dispatches correctly', () => {
    const history = [makeBanHistoryItem()];
    Dispatch.banHistory('Ira', history);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.banHistory('Ira', history));
  });

  it('warnHistory dispatches correctly', () => {
    const history = [makeWarnHistoryItem()];
    Dispatch.warnHistory('Jack', history);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.warnHistory('Jack', history));
  });

  it('warnListOptions dispatches correctly', () => {
    const list = [makeWarnListItem()];
    Dispatch.warnListOptions(list);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.warnListOptions(list));
  });

  it('warnUser dispatches correctly', () => {
    Dispatch.warnUser('Kelly');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.warnUser('Kelly'));
  });

  it('grantReplayAccess dispatches correctly', () => {
    Dispatch.grantReplayAccess(7, 'Moe');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.grantReplayAccess(7, 'Moe'));
  });

  it('forceActivateUser dispatches correctly', () => {
    Dispatch.forceActivateUser('Ned', 'Moe');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.forceActivateUser('Ned', 'Moe'));
  });

  it('getAdminNotes dispatches correctly', () => {
    Dispatch.getAdminNotes('Ned', 'notes');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.getAdminNotes('Ned', 'notes'));
  });

  it('updateAdminNotes dispatches correctly', () => {
    Dispatch.updateAdminNotes('Ned', 'updated');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateAdminNotes('Ned', 'updated'));
  });

  it('replayList dispatches correctly', () => {
    const list = [makeReplayMatch()];
    Dispatch.replayList(list);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.replayList(list));
  });

  it('replayAdded dispatches correctly', () => {
    const match = makeReplayMatch();
    Dispatch.replayAdded(match);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.replayAdded(match));
  });

  it('replayModifyMatch dispatches correctly', () => {
    Dispatch.replayModifyMatch(5, true);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.replayModifyMatch(5, true));
  });

  it('replayDeleteMatch dispatches correctly', () => {
    Dispatch.replayDeleteMatch(5);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.replayDeleteMatch(5));
  });

  it('backendDecks dispatches correctly', () => {
    const deckList = makeDeckList();
    Dispatch.backendDecks(deckList);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.backendDecks(deckList));
  });

  it('deckNewDir dispatches correctly', () => {
    Dispatch.deckNewDir('a/b', 'newFolder');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.deckNewDir('a/b', 'newFolder'));
  });

  it('deckDelDir dispatches correctly', () => {
    Dispatch.deckDelDir('a/b');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.deckDelDir('a/b'));
  });

  it('deckUpload dispatches correctly', () => {
    const treeItem = makeDeckTreeItem();
    Dispatch.deckUpload('a/b', treeItem);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.deckUpload('a/b', treeItem));
  });

  it('deckDelete dispatches correctly', () => {
    Dispatch.deckDelete(42);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.deckDelete(42));
  });

  it('gamesOfUser dispatches correctly', () => {
    const games = [makeGame({ gameId: 1 })];
    const gametypeMap = { 1: 'Standard' };
    Dispatch.gamesOfUser('alice', games, gametypeMap);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.gamesOfUser('alice', games, gametypeMap));
  });
});
