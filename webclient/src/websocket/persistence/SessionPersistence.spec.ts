jest.mock('store', () => ({
  ServerDispatch: {
    initialized: jest.fn(),
    clearStore: jest.fn(),
    loginSuccessful: jest.fn(),
    loginFailed: jest.fn(),
    connectionClosed: jest.fn(),
    connectionFailed: jest.fn(),
    testConnectionSuccessful: jest.fn(),
    testConnectionFailed: jest.fn(),
    updateBuddyList: jest.fn(),
    addToBuddyList: jest.fn(),
    removeFromBuddyList: jest.fn(),
    updateIgnoreList: jest.fn(),
    addToIgnoreList: jest.fn(),
    removeFromIgnoreList: jest.fn(),
    updateInfo: jest.fn(),
    updateStatus: jest.fn(),
    updateUser: jest.fn(),
    updateUsers: jest.fn(),
    userJoined: jest.fn(),
    userLeft: jest.fn(),
    serverMessage: jest.fn(),
    accountAwaitingActivation: jest.fn(),
    accountActivationSuccess: jest.fn(),
    accountActivationFailed: jest.fn(),
    registrationRequiresEmail: jest.fn(),
    registrationSuccess: jest.fn(),
    registrationFailed: jest.fn(),
    registrationEmailError: jest.fn(),
    registrationPasswordError: jest.fn(),
    registrationUserNameError: jest.fn(),
    resetPasswordChallenge: jest.fn(),
    resetPassword: jest.fn(),
    resetPasswordSuccess: jest.fn(),
    resetPasswordFailed: jest.fn(),
    accountPasswordChange: jest.fn(),
    accountEditChanged: jest.fn(),
    accountImageChanged: jest.fn(),
    directMessageSent: jest.fn(),
    getUserInfo: jest.fn(),
    notifyUser: jest.fn(),
    serverShutdown: jest.fn(),
    userMessage: jest.fn(),
    addToList: jest.fn(),
    removeFromList: jest.fn(),
    deckDelete: jest.fn(),
    backendDecks: jest.fn(),
    deckUpload: jest.fn(),
    deckNewDir: jest.fn(),
    deckDelDir: jest.fn(),
    replayList: jest.fn(),
    replayAdded: jest.fn(),
    replayModifyMatch: jest.fn(),
    replayDeleteMatch: jest.fn(),
  },
}));

jest.mock('websocket/utils', () => ({
  sanitizeHtml: jest.fn((msg: string) => `sanitized:${msg}`),
}));

jest.mock('../utils/NormalizeService', () => ({
  __esModule: true,
  default: {
    normalizeBannedUserError: jest.fn((r: string, t: number) => `banned:${r}:${t}`),
  },
}));

import { SessionPersistence } from './SessionPersistence';
import { ServerDispatch } from 'store';
import { sanitizeHtml } from 'websocket/utils';
import NormalizeService from '../utils/NormalizeService';
import { StatusEnum } from 'types';

beforeEach(() => {
  jest.clearAllMocks();
  (sanitizeHtml as jest.Mock).mockImplementation((msg: string) => `sanitized:${msg}`);
  (NormalizeService.normalizeBannedUserError as jest.Mock).mockImplementation(
    (r: string, t: number) => `banned:${r}:${t}`
  );
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
    const opts = { userName: 'alice' } as any;
    SessionPersistence.loginSuccessful(opts);
    expect(ServerDispatch.loginSuccessful).toHaveBeenCalledWith(opts);
  });

  it('loginFailed -> ServerDispatch.loginFailed', () => {
    SessionPersistence.loginFailed();
    expect(ServerDispatch.loginFailed).toHaveBeenCalled();
  });

  it('connectionClosed passes reason', () => {
    SessionPersistence.connectionClosed(3);
    expect(ServerDispatch.connectionClosed).toHaveBeenCalledWith(3);
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
    const user = { name: 'bob' } as any;
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
    const user = { name: 'bob' } as any;
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

  it('updateStatus dispatches status and calls connectionClosed when DISCONNECTED', () => {
    SessionPersistence.updateStatus(StatusEnum.DISCONNECTED, 'bye');
    expect(ServerDispatch.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, 'bye');
    expect(ServerDispatch.connectionClosed).toHaveBeenCalledWith(StatusEnum.DISCONNECTED);
  });

  it('updateStatus does not call connectionClosed when not DISCONNECTED', () => {
    SessionPersistence.updateStatus(StatusEnum.CONNECTED, 'hi');
    expect(ServerDispatch.connectionClosed).not.toHaveBeenCalled();
  });

  it('updateUser passes user', () => {
    const user = { name: 'alice' } as any;
    SessionPersistence.updateUser(user);
    expect(ServerDispatch.updateUser).toHaveBeenCalledWith(user);
  });

  it('updateUsers passes users array', () => {
    SessionPersistence.updateUsers([]);
    expect(ServerDispatch.updateUsers).toHaveBeenCalledWith([]);
  });

  it('userJoined passes user', () => {
    const user = { name: 'carol' } as any;
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
    const opts = { userName: 'u' } as any;
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

  it('registrationFailed normalizes ban error when endTime is given', () => {
    SessionPersistence.registrationFailed('reason', 999);
    expect(NormalizeService.normalizeBannedUserError).toHaveBeenCalledWith('reason', 999);
    expect(ServerDispatch.registrationFailed).toHaveBeenCalledWith('banned:reason:999');
  });

  it('registrationFailed uses reason directly when no endTime', () => {
    SessionPersistence.registrationFailed('plain reason');
    expect(ServerDispatch.registrationFailed).toHaveBeenCalledWith('plain reason');
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

  it('directMessageSent passes userName and message', () => {
    SessionPersistence.directMessageSent('bob', 'hi');
    expect(ServerDispatch.directMessageSent).toHaveBeenCalledWith('bob', 'hi');
  });

  it('getUserInfo passes userInfo', () => {
    const user = { name: 'u' } as any;
    SessionPersistence.getUserInfo(user);
    expect(ServerDispatch.getUserInfo).toHaveBeenCalledWith(user);
  });

  it('getGamesOfUser logs to console', () => {
    const spy = jest.spyOn(console, 'log').mockImplementation(() => {});
    SessionPersistence.getGamesOfUser('user1', {});
    expect(spy).toHaveBeenCalled();
    spy.mockRestore();
  });

  it('gameJoined logs to console', () => {
    const spy = jest.spyOn(console, 'log').mockImplementation(() => {});
    SessionPersistence.gameJoined({ gameInfo: {} } as any);
    expect(spy).toHaveBeenCalled();
    spy.mockRestore();
  });

  it('notifyUser passes notification', () => {
    const notif = { type: 1 } as any;
    SessionPersistence.notifyUser(notif);
    expect(ServerDispatch.notifyUser).toHaveBeenCalledWith(notif);
  });

  it('playerPropertiesChanged logs to console', () => {
    const spy = jest.spyOn(console, 'log').mockImplementation(() => {});
    SessionPersistence.playerPropertiesChanged({} as any);
    expect(spy).toHaveBeenCalled();
    spy.mockRestore();
  });

  it('serverShutdown passes data', () => {
    const data = { gracePeriod: 5 } as any;
    SessionPersistence.serverShutdown(data);
    expect(ServerDispatch.serverShutdown).toHaveBeenCalledWith(data);
  });

  it('userMessage passes messageData', () => {
    const msg = { message: 'hello' } as any;
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
    SessionPersistence.updateServerDecks({ folders: [] } as any);
    expect(ServerDispatch.backendDecks).toHaveBeenCalled();
  });

  it('uploadServerDeck passes path and treeItem', () => {
    SessionPersistence.uploadServerDeck('/path', { id: 1 } as any);
    expect(ServerDispatch.deckUpload).toHaveBeenCalledWith('/path', { id: 1 });
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
    const match = { gameId: 1 } as any;
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
});
