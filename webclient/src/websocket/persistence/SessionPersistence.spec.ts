vi.mock('@app/store', () => ({
  ServerDispatch: {
    initialized: vi.fn(),
    connectionAttempted: vi.fn(),
    clearStore: vi.fn(),
    loginSuccessful: vi.fn(),
    loginFailed: vi.fn(),
    connectionFailed: vi.fn(),
    testConnectionSuccessful: vi.fn(),
    testConnectionFailed: vi.fn(),
    updateBuddyList: vi.fn(),
    addToBuddyList: vi.fn(),
    removeFromBuddyList: vi.fn(),
    updateIgnoreList: vi.fn(),
    addToIgnoreList: vi.fn(),
    removeFromIgnoreList: vi.fn(),
    updateInfo: vi.fn(),
    updateStatus: vi.fn(),
    updateUser: vi.fn(),
    updateUsers: vi.fn(),
    userJoined: vi.fn(),
    userLeft: vi.fn(),
    serverMessage: vi.fn(),
    accountAwaitingActivation: vi.fn(),
    accountActivationSuccess: vi.fn(),
    accountActivationFailed: vi.fn(),
    registrationRequiresEmail: vi.fn(),
    registrationSuccess: vi.fn(),
    registrationFailed: vi.fn(),
    registrationEmailError: vi.fn(),
    registrationPasswordError: vi.fn(),
    registrationUserNameError: vi.fn(),
    resetPasswordChallenge: vi.fn(),
    resetPassword: vi.fn(),
    resetPasswordSuccess: vi.fn(),
    resetPasswordFailed: vi.fn(),
    accountPasswordChange: vi.fn(),
    accountEditChanged: vi.fn(),
    accountImageChanged: vi.fn(),
    getUserInfo: vi.fn(),
    notifyUser: vi.fn(),
    serverShutdown: vi.fn(),
    userMessage: vi.fn(),
    addToList: vi.fn(),
    removeFromList: vi.fn(),
    deckDelete: vi.fn(),
    backendDecks: vi.fn(),
    deckUpload: vi.fn(),
    deckNewDir: vi.fn(),
    deckDelDir: vi.fn(),
    replayList: vi.fn(),
    replayAdded: vi.fn(),
    replayModifyMatch: vi.fn(),
    replayDeleteMatch: vi.fn(),
    gamesOfUser: vi.fn(),
  },
  GameDispatch: {
    gameJoined: vi.fn(),
    playerPropertiesChanged: vi.fn(),
  },
}));

vi.mock('../utils', () => ({
  sanitizeHtml: vi.fn((msg: string) => `sanitized:${msg}`),
}));

import { SessionPersistence } from './SessionPersistence';
import { ServerDispatch, GameDispatch } from '@app/store';
import { sanitizeHtml } from '../utils';
import { App, Data, Enriched } from '@app/types';
import { create } from '@bufbuild/protobuf';

import { Mock } from 'vitest';

beforeEach(() => {
  (sanitizeHtml as Mock).mockImplementation((msg: string) => `sanitized:${msg}`);
});

describe('SessionPersistence', () => {
  it('initialized -> ServerDispatch.initialized', () => {
    SessionPersistence.initialized();
    expect(ServerDispatch.initialized).toHaveBeenCalled();
  });

  it('clearStore -> ServerDispatch.clearStore', () => {
    SessionPersistence.clearStore();
    expect(ServerDispatch.clearStore).toHaveBeenCalled();
  });

  it('loginSuccessful passes options', () => {
    const opts: Enriched.LoginSuccessContext = { hashedPassword: 'hash' };
    SessionPersistence.loginSuccessful(opts);
    expect(ServerDispatch.loginSuccessful).toHaveBeenCalledWith(opts);
  });

  it('loginFailed -> ServerDispatch.loginFailed', () => {
    SessionPersistence.loginFailed();
    expect(ServerDispatch.loginFailed).toHaveBeenCalled();
  });

  it('connectionFailed -> ServerDispatch.connectionFailed', () => {
    SessionPersistence.connectionFailed();
    expect(ServerDispatch.connectionFailed).toHaveBeenCalled();
  });

  it('testConnectionSuccessful -> ServerDispatch.testConnectionSuccessful', () => {
    SessionPersistence.testConnectionSuccessful();
    expect(ServerDispatch.testConnectionSuccessful).toHaveBeenCalled();
  });

  it('testConnectionFailed -> ServerDispatch.testConnectionFailed', () => {
    SessionPersistence.testConnectionFailed();
    expect(ServerDispatch.testConnectionFailed).toHaveBeenCalled();
  });

  it('updateBuddyList passes list', () => {
    SessionPersistence.updateBuddyList(['user']);
    expect(ServerDispatch.updateBuddyList).toHaveBeenCalledWith(['user']);
  });

  it('addToBuddyList passes user', () => {
    const user = create(Data.ServerInfo_UserSchema, { name: 'bob' });
    SessionPersistence.addToBuddyList(user);
    expect(ServerDispatch.addToBuddyList).toHaveBeenCalledWith(user);
  });

  it('removeFromBuddyList passes userName', () => {
    SessionPersistence.removeFromBuddyList('bob');
    expect(ServerDispatch.removeFromBuddyList).toHaveBeenCalledWith('bob');
  });

  it('updateIgnoreList passes list', () => {
    SessionPersistence.updateIgnoreList(['user']);
    expect(ServerDispatch.updateIgnoreList).toHaveBeenCalledWith(['user']);
  });

  it('addToIgnoreList passes user', () => {
    const user = create(Data.ServerInfo_UserSchema, { name: 'bob' });
    SessionPersistence.addToIgnoreList(user);
    expect(ServerDispatch.addToIgnoreList).toHaveBeenCalledWith(user);
  });

  it('removeFromIgnoreList passes userName', () => {
    SessionPersistence.removeFromIgnoreList('bob');
    expect(ServerDispatch.removeFromIgnoreList).toHaveBeenCalledWith('bob');
  });

  it('updateInfo passes name and version', () => {
    SessionPersistence.updateInfo('Server', '1.0');
    expect(ServerDispatch.updateInfo).toHaveBeenCalledWith('Server', '1.0');
  });

  it('updateStatus passes state and description', () => {
    SessionPersistence.updateStatus(App.StatusEnum.DISCONNECTED, 'bye');
    expect(ServerDispatch.updateStatus).toHaveBeenCalledWith(App.StatusEnum.DISCONNECTED, 'bye');
  });

  it('updateUser passes user', () => {
    const user = create(Data.ServerInfo_UserSchema, { name: 'alice' });
    SessionPersistence.updateUser(user);
    expect(ServerDispatch.updateUser).toHaveBeenCalledWith(user);
  });

  it('updateUsers passes users array', () => {
    SessionPersistence.updateUsers([]);
    expect(ServerDispatch.updateUsers).toHaveBeenCalledWith([]);
  });

  it('userJoined passes user', () => {
    const user = create(Data.ServerInfo_UserSchema, { name: 'carol' });
    SessionPersistence.userJoined(user);
    expect(ServerDispatch.userJoined).toHaveBeenCalledWith(user);
  });

  it('userLeft passes userName', () => {
    SessionPersistence.userLeft('carol');
    expect(ServerDispatch.userLeft).toHaveBeenCalledWith('carol');
  });

  it('serverMessage sanitizes message', () => {
    SessionPersistence.serverMessage('<b>hello</b>');
    expect(sanitizeHtml).toHaveBeenCalledWith('<b>hello</b>');
    expect(ServerDispatch.serverMessage).toHaveBeenCalledWith('sanitized:<b>hello</b>');
  });

  it('accountAwaitingActivation passes options', () => {
    const opts: Enriched.PendingActivationContext = { host: 'h', port: '1', userName: 'u' };
    SessionPersistence.accountAwaitingActivation(opts);
    expect(ServerDispatch.accountAwaitingActivation).toHaveBeenCalledWith(opts);
  });

  it('accountActivationSuccess -> ServerDispatch.accountActivationSuccess', () => {
    SessionPersistence.accountActivationSuccess();
    expect(ServerDispatch.accountActivationSuccess).toHaveBeenCalled();
  });

  it('accountActivationFailed -> ServerDispatch.accountActivationFailed', () => {
    SessionPersistence.accountActivationFailed();
    expect(ServerDispatch.accountActivationFailed).toHaveBeenCalled();
  });

  it('registrationRequiresEmail -> ServerDispatch.registrationRequiresEmail', () => {
    SessionPersistence.registrationRequiresEmail();
    expect(ServerDispatch.registrationRequiresEmail).toHaveBeenCalled();
  });

  it('registrationSuccess -> ServerDispatch.registrationSuccess', () => {
    SessionPersistence.registrationSuccess();
    expect(ServerDispatch.registrationSuccess).toHaveBeenCalled();
  });

  it('registrationFailed passes reason and endTime to ServerDispatch', () => {
    SessionPersistence.registrationFailed('reason', 999);
    expect(ServerDispatch.registrationFailed).toHaveBeenCalledWith('reason', 999);
  });

  it('registrationFailed passes reason only when no endTime', () => {
    SessionPersistence.registrationFailed('plain reason');
    expect(ServerDispatch.registrationFailed).toHaveBeenCalledWith('plain reason', undefined);
  });

  it('registrationEmailError passes error', () => {
    SessionPersistence.registrationEmailError('bad email');
    expect(ServerDispatch.registrationEmailError).toHaveBeenCalledWith('bad email');
  });

  it('registrationPasswordError passes error', () => {
    SessionPersistence.registrationPasswordError('short password');
    expect(ServerDispatch.registrationPasswordError).toHaveBeenCalledWith('short password');
  });

  it('registrationUserNameError passes error', () => {
    SessionPersistence.registrationUserNameError('taken');
    expect(ServerDispatch.registrationUserNameError).toHaveBeenCalledWith('taken');
  });

  it('resetPasswordChallenge -> ServerDispatch.resetPasswordChallenge', () => {
    SessionPersistence.resetPasswordChallenge();
    expect(ServerDispatch.resetPasswordChallenge).toHaveBeenCalled();
  });

  it('resetPassword -> ServerDispatch.resetPassword', () => {
    SessionPersistence.resetPassword();
    expect(ServerDispatch.resetPassword).toHaveBeenCalled();
  });

  it('resetPasswordSuccess -> ServerDispatch.resetPasswordSuccess', () => {
    SessionPersistence.resetPasswordSuccess();
    expect(ServerDispatch.resetPasswordSuccess).toHaveBeenCalled();
  });

  it('resetPasswordFailed -> ServerDispatch.resetPasswordFailed', () => {
    SessionPersistence.resetPasswordFailed();
    expect(ServerDispatch.resetPasswordFailed).toHaveBeenCalled();
  });

  it('accountPasswordChange -> ServerDispatch.accountPasswordChange', () => {
    SessionPersistence.accountPasswordChange();
    expect(ServerDispatch.accountPasswordChange).toHaveBeenCalled();
  });

  it('accountEditChanged passes fields', () => {
    SessionPersistence.accountEditChanged('Alice', 'a@b.com', 'US');
    expect(ServerDispatch.accountEditChanged).toHaveBeenCalledWith({ realName: 'Alice', email: 'a@b.com', country: 'US' });
  });

  it('accountImageChanged passes avatarBmp', () => {
    const buf = new Uint8Array([1, 2, 3]);
    SessionPersistence.accountImageChanged(buf);
    expect(ServerDispatch.accountImageChanged).toHaveBeenCalledWith({ avatarBmp: buf });
  });

  it('getUserInfo passes userInfo', () => {
    const user = create(Data.ServerInfo_UserSchema, { name: 'u' });
    SessionPersistence.getUserInfo(user);
    expect(ServerDispatch.getUserInfo).toHaveBeenCalledWith(user);
  });

  it('getGamesOfUser passes response to ServerDispatch', () => {
    const response = create(Data.Response_GetGamesOfUserSchema, {
      roomList: [create(Data.ServerInfo_RoomSchema, {
        gametypeList: [create(Data.ServerInfo_GameTypeSchema, { gameTypeId: 1, description: 'Standard' })]
      })],
      gameList: [create(Data.ServerInfo_GameSchema, { gameId: 5 })],
    });
    SessionPersistence.getGamesOfUser('alice', response);
    expect(ServerDispatch.gamesOfUser).toHaveBeenCalledWith('alice', response);
  });

  it('getGamesOfUser handles empty response', () => {
    const emptyResponse = create(Data.Response_GetGamesOfUserSchema, {});
    SessionPersistence.getGamesOfUser('alice', emptyResponse);
    expect(ServerDispatch.gamesOfUser).toHaveBeenCalledWith('alice', emptyResponse);
  });

  it('gameJoined dispatches raw event via GameDispatch.gameJoined', () => {
    const gameInfo = create(Data.ServerInfo_GameSchema, { gameId: 10, roomId: 2, description: 'test', started: false });
    const data = create(Data.Event_GameJoinedSchema, { gameInfo, hostId: 3, playerId: 4, spectator: false, judge: false, resuming: true });
    SessionPersistence.gameJoined(data);
    expect(GameDispatch.gameJoined).toHaveBeenCalledWith(data);
  });

  it('notifyUser passes notification', () => {
    const notif = create(Data.Event_NotifyUserSchema, { type: 1 });
    SessionPersistence.notifyUser(notif);
    expect(ServerDispatch.notifyUser).toHaveBeenCalledWith(notif);
  });

  it('playerPropertiesChanged dispatches via GameDispatch', () => {
    const props = create(Data.ServerInfo_PlayerPropertiesSchema, { pingTime: 100 });
    SessionPersistence.playerPropertiesChanged(5, 1, create(Data.Event_PlayerPropertiesChangedSchema, { playerProperties: props }));
    expect(GameDispatch.playerPropertiesChanged).toHaveBeenCalledWith(5, 1, props);
  });

  it('serverShutdown passes data', () => {
    const data = create(Data.Event_ServerShutdownSchema, { gracePeriod: 5 });
    SessionPersistence.serverShutdown(data);
    expect(ServerDispatch.serverShutdown).toHaveBeenCalledWith(data);
  });

  it('userMessage passes messageData', () => {
    const msg = create(Data.Event_UserMessageSchema, { message: 'hello' });
    SessionPersistence.userMessage(msg);
    expect(ServerDispatch.userMessage).toHaveBeenCalledWith(msg);
  });

  it('addToList passes list and userName', () => {
    SessionPersistence.addToList('buddy', 'alice');
    expect(ServerDispatch.addToList).toHaveBeenCalledWith('buddy', 'alice');
  });

  it('removeFromList passes list and userName', () => {
    SessionPersistence.removeFromList('ignore', 'bob');
    expect(ServerDispatch.removeFromList).toHaveBeenCalledWith('ignore', 'bob');
  });

  it('deleteServerDeck passes deckId', () => {
    SessionPersistence.deleteServerDeck(42);
    expect(ServerDispatch.deckDelete).toHaveBeenCalledWith(42);
  });

  it('updateServerDecks passes deckList', () => {
    SessionPersistence.updateServerDecks(create(Data.Response_DeckListSchema, { folders: [] }));
    expect(ServerDispatch.backendDecks).toHaveBeenCalled();
  });

  it('uploadServerDeck passes path and treeItem', () => {
    const treeItem = create(Data.ServerInfo_DeckStorage_TreeItemSchema, { id: 1 });
    SessionPersistence.uploadServerDeck('/path', treeItem);
    expect(ServerDispatch.deckUpload).toHaveBeenCalledWith('/path', treeItem);
  });

  it('createServerDeckDir passes path and dirName', () => {
    SessionPersistence.createServerDeckDir('/path', 'newdir');
    expect(ServerDispatch.deckNewDir).toHaveBeenCalledWith('/path', 'newdir');
  });

  it('deleteServerDeckDir passes path', () => {
    SessionPersistence.deleteServerDeckDir('/path');
    expect(ServerDispatch.deckDelDir).toHaveBeenCalledWith('/path');
  });

  it('replayList passes matchList', () => {
    SessionPersistence.replayList([]);
    expect(ServerDispatch.replayList).toHaveBeenCalledWith([]);
  });

  it('replayAdded passes matchInfo', () => {
    const match = create(Data.ServerInfo_ReplayMatchSchema, { gameId: 1 });
    SessionPersistence.replayAdded(match);
    expect(ServerDispatch.replayAdded).toHaveBeenCalledWith(match);
  });

  it('replayModifyMatch passes gameId and doNotHide', () => {
    SessionPersistence.replayModifyMatch(7, true);
    expect(ServerDispatch.replayModifyMatch).toHaveBeenCalledWith(7, true);
  });

  it('replayDeleteMatch passes gameId', () => {
    SessionPersistence.replayDeleteMatch(7);
    expect(ServerDispatch.replayDeleteMatch).toHaveBeenCalledWith(7);
  });

  it('connectionAttempted delegates to ServerDispatch', () => {
    SessionPersistence.connectionAttempted();
    expect(ServerDispatch.connectionAttempted).toHaveBeenCalled();
  });

  it('playerPropertiesChanged does nothing when payload has no playerProperties', () => {
    SessionPersistence.playerPropertiesChanged(5, 1, create(Data.Event_PlayerPropertiesChangedSchema, {}));
    expect(GameDispatch.playerPropertiesChanged).not.toHaveBeenCalled();
  });

  it('getGamesOfUser handles rooms with missing gametypeList', () => {
    const room = create(Data.ServerInfo_RoomSchema, {});
    const game = create(Data.ServerInfo_GameSchema, { gameId: 5 });
    const response = create(Data.Response_GetGamesOfUserSchema, { roomList: [room], gameList: [game] });
    SessionPersistence.getGamesOfUser('alice', response);
    expect(ServerDispatch.gamesOfUser).toHaveBeenCalledWith('alice', response);
  });
});
