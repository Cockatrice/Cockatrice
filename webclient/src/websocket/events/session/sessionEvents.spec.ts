// Tests for simple session events that delegate 1:1 to SessionPersistence
// or RoomPersistence with minimal logic.

vi.mock('../../persistence', () => ({
  SessionPersistence: {
    gameJoined: vi.fn(),
    notifyUser: vi.fn(),
    replayAdded: vi.fn(),
    serverMessage: vi.fn(),
    serverShutdown: vi.fn(),
    updateUsers: vi.fn(),
    updateInfo: vi.fn(),
    userJoined: vi.fn(),
    userLeft: vi.fn(),
    userMessage: vi.fn(),
    addToBuddyList: vi.fn(),
    addToIgnoreList: vi.fn(),
    removeFromBuddyList: vi.fn(),
    removeFromIgnoreList: vi.fn(),
    playerPropertiesChanged: vi.fn(),
  },
  RoomPersistence: {
    updateRooms: vi.fn(),
  },
}));

vi.mock('../../WebClient', () => ({
  __esModule: true,
  default: {
    options: {},
  },
}));

vi.mock('../../config', () => ({
  CLIENT_OPTIONS: { autojoinrooms: false },
  PROTOCOL_VERSION: 14,
}));

vi.mock('../../commands/session', () => ({
  joinRoom: vi.fn(),
  updateStatus: vi.fn(),
  disconnect: vi.fn(),
  login: vi.fn(),
  register: vi.fn(),
  activate: vi.fn(),
  requestPasswordSalt: vi.fn(),
  forgotPasswordRequest: vi.fn(),
  forgotPasswordChallenge: vi.fn(),
  forgotPasswordReset: vi.fn(),
}));

vi.mock('../../utils', () => ({
  generateSalt: vi.fn().mockReturnValue('newSalt'),
  passwordSaltSupported: vi.fn().mockReturnValue(0),
}));

import { App, Data, Enriched } from '@app/types';
import { create } from '@bufbuild/protobuf';

import { SessionPersistence, RoomPersistence } from '../../persistence';
import webClient from '../../WebClient';
import * as Config from '../../config';
import * as SessionCmds from '../../commands/session';
import * as Utils from '../../utils';
import { gameJoined } from './gameJoined';
import { notifyUser } from './notifyUser';
import { replayAdded } from './replayAdded';
import { serverCompleteList } from './serverCompleteList';
import { serverMessage } from './serverMessage';
import { serverShutdown } from './serverShutdown';
import { userJoined } from './userJoined';
import { userLeft } from './userLeft';
import { userMessage } from './userMessage';
import { addToList } from './addToList';
import { removeFromList } from './removeFromList';
import { listRooms } from './listRooms';
import { connectionClosed } from './connectionClosed';
import { serverIdentification } from './serverIdentification';
import { Mock } from 'vitest';

const ConfigMock = Config as { -readonly [K in keyof typeof Config]: (typeof Config)[K] };

beforeEach(() => {
  (Utils.generateSalt as Mock).mockReturnValue('newSalt');
  (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------
// gameJoined
// ----------------------------------------------------------------
describe('gameJoined', () => {

  it('calls SessionPersistence.gameJoined', () => {
    const data = create(Data.Event_GameJoinedSchema, { playerId: 1 });
    gameJoined(data);
    expect(SessionPersistence.gameJoined).toHaveBeenCalledWith(data);
  });
});

// ----------------------------------------------------------------
// notifyUser
// ----------------------------------------------------------------
describe('notifyUser', () => {

  it('calls SessionPersistence.notifyUser', () => {
    const data = create(Data.Event_NotifyUserSchema, { warningReason: 'yo' });
    notifyUser(data);
    expect(SessionPersistence.notifyUser).toHaveBeenCalledWith(data);
  });
});

// ----------------------------------------------------------------
// replayAdded
// ----------------------------------------------------------------
describe('replayAdded', () => {

  it('calls SessionPersistence.replayAdded with matchInfo', () => {
    const data = create(Data.Event_ReplayAddedSchema, {
      matchInfo: create(Data.ServerInfo_ReplayMatchSchema, { gameId: 42 }),
    });
    replayAdded(data);
    expect(SessionPersistence.replayAdded).toHaveBeenCalledWith(data.matchInfo);
  });
});

// ----------------------------------------------------------------
// serverCompleteList
// ----------------------------------------------------------------
describe('serverCompleteList', () => {

  it('calls SessionPersistence.updateUsers and RoomPersistence.updateRooms', () => {
    const data = create(Data.Event_ServerCompleteListSchema, { userList: [], roomList: [] });
    serverCompleteList(data);
    expect(SessionPersistence.updateUsers).toHaveBeenCalledWith(data.userList);
    expect(RoomPersistence.updateRooms).toHaveBeenCalledWith(data.roomList);
  });
});

// ----------------------------------------------------------------
// serverMessage
// ----------------------------------------------------------------
describe('serverMessage', () => {

  it('calls SessionPersistence.serverMessage with message', () => {
    serverMessage(create(Data.Event_ServerMessageSchema, { message: 'hello server' }));
    expect(SessionPersistence.serverMessage).toHaveBeenCalledWith('hello server');
  });
});

// ----------------------------------------------------------------
// serverShutdown
// ----------------------------------------------------------------
describe('serverShutdown', () => {

  it('calls SessionPersistence.serverShutdown', () => {
    const payload = create(Data.Event_ServerShutdownSchema, { reason: 'maintenance' });
    serverShutdown(payload);
    expect(SessionPersistence.serverShutdown).toHaveBeenCalledWith(payload);
  });
});

// ----------------------------------------------------------------
// userJoined
// ----------------------------------------------------------------
describe('userJoined', () => {

  it('calls SessionPersistence.userJoined with userInfo', () => {
    const data = create(Data.Event_UserJoinedSchema, {
      userInfo: create(Data.ServerInfo_UserSchema, { name: 'alice' }),
    });
    userJoined(data);
    expect(SessionPersistence.userJoined).toHaveBeenCalledWith(data.userInfo);
  });
});

// ----------------------------------------------------------------
// userLeft
// ----------------------------------------------------------------
describe('userLeft', () => {

  it('calls SessionPersistence.userLeft with name', () => {
    userLeft(create(Data.Event_UserLeftSchema, { name: 'bob' }));
    expect(SessionPersistence.userLeft).toHaveBeenCalledWith('bob');
  });
});

// ----------------------------------------------------------------
// userMessage
// ----------------------------------------------------------------
describe('userMessage', () => {

  it('calls SessionPersistence.userMessage', () => {
    const payload = create(Data.Event_UserMessageSchema, { senderName: 'alice', message: 'hi' });
    userMessage(payload);
    expect(SessionPersistence.userMessage).toHaveBeenCalledWith(payload);
  });
});

// ----------------------------------------------------------------
// addToList
// ----------------------------------------------------------------
describe('addToList', () => {
  let logSpy: ReturnType<typeof vi.spyOn>;
  beforeEach(() => {
    logSpy = vi.spyOn(console, 'log').mockImplementation(() => {});
  });

  it('buddy list → addToBuddyList', () => {
    const data = create(Data.Event_AddToListSchema, {
      listName: 'buddy',
      userInfo: create(Data.ServerInfo_UserSchema, { name: 'alice' }),
    });
    addToList(data);
    expect(SessionPersistence.addToBuddyList).toHaveBeenCalledWith(data.userInfo);
  });

  it('ignore list → addToIgnoreList', () => {
    const data = create(Data.Event_AddToListSchema, {
      listName: 'ignore',
      userInfo: create(Data.ServerInfo_UserSchema, { name: 'bob' }),
    });
    addToList(data);
    expect(SessionPersistence.addToIgnoreList).toHaveBeenCalledWith(data.userInfo);
  });

  it('unknown list → console.log', () => {
    addToList(create(Data.Event_AddToListSchema, { listName: 'unknown' }));
    expect(logSpy).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// removeFromList
// ----------------------------------------------------------------
describe('removeFromList', () => {

  it('buddy list → removeFromBuddyList', () => {
    removeFromList(create(Data.Event_RemoveFromListSchema, { listName: 'buddy', userName: 'alice' }));
    expect(SessionPersistence.removeFromBuddyList).toHaveBeenCalledWith('alice');
  });

  it('ignore list → removeFromIgnoreList', () => {
    removeFromList(create(Data.Event_RemoveFromListSchema, { listName: 'ignore', userName: 'bob' }));
    expect(SessionPersistence.removeFromIgnoreList).toHaveBeenCalledWith('bob');
  });

  it('unknown list → console.log', () => {
    const logSpy = vi.spyOn(console, 'log').mockImplementation(() => {});
    removeFromList(create(Data.Event_RemoveFromListSchema, { listName: 'other', userName: 'x' }));
    expect(logSpy).toHaveBeenCalled();
    logSpy.mockRestore();
  });
});

// ----------------------------------------------------------------
// listRooms
// ----------------------------------------------------------------
describe('listRooms', () => {

  it('calls RoomPersistence.updateRooms', () => {
    listRooms(create(Data.Event_ListRoomsSchema, { roomList: [] }));
    expect(RoomPersistence.updateRooms).toHaveBeenCalledWith([]);
  });

  it('does not call joinRoom when autojoinrooms is false', () => {
    ConfigMock.CLIENT_OPTIONS = { autojoinrooms: false };
    listRooms(create(Data.Event_ListRoomsSchema, {
      roomList: [create(Data.ServerInfo_RoomSchema, { autoJoin: true, roomId: 1 })]
    }));
    expect(SessionCmds.joinRoom).not.toHaveBeenCalled();
  });

  it('calls joinRoom for autoJoin rooms when autojoinrooms is true', () => {
    ConfigMock.CLIENT_OPTIONS = { autojoinrooms: true };
    listRooms(create(Data.Event_ListRoomsSchema, {
      roomList: [
        create(Data.ServerInfo_RoomSchema, { autoJoin: true, roomId: 2 }),
        create(Data.ServerInfo_RoomSchema, { autoJoin: false, roomId: 3 })
      ]
    }));
    expect(SessionCmds.joinRoom).toHaveBeenCalledTimes(1);
    expect(SessionCmds.joinRoom).toHaveBeenCalledWith(2);
  });
});

// ----------------------------------------------------------------
// connectionClosed
// ----------------------------------------------------------------
describe('connectionClosed', () => {

  it('uses reasonStr when provided', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: 0, reasonStr: 'custom' }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'custom');
  });

  it('USER_LIMIT_REACHED → specific message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.USER_LIMIT_REACHED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(
      expect.anything(),
      expect.stringContaining('maximum user capacity')
    );
  });

  it('TOO_MANY_CONNECTIONS → specific message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.TOO_MANY_CONNECTIONS }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('too many concurrent'));
  });

  it('BANNED → specific message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.BANNED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('DEMOTED → specific message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.DEMOTED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('demoted'));
  });

  it('SERVER_SHUTDOWN → specific message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.SERVER_SHUTDOWN }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('shutdown'));
  });

  it('USERNAMEINVALID → specific message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.USERNAMEINVALID }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('username'));
  });

  it('LOGGEDINELSEWERE → specific message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.LOGGEDINELSEWERE }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('logged out'));
  });

  it('OTHER → "Unknown reason"', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.OTHER }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'Unknown reason');
  });

  it('BANNED with valid positive endTime → shows formatted date', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, {
      reason: Data.Event_ConnectionClosed_CloseReason.BANNED, endTime: 1700000000,
    }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(
      expect.anything(),
      expect.stringContaining('You are banned until')
    );
  });

  it('BANNED with endTime = 0 → shows generic banned message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.BANNED, endTime: 0 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = -1 → shows generic banned message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.BANNED, endTime: -1 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = NaN → shows generic banned message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, { reason: Data.Event_ConnectionClosed_CloseReason.BANNED, endTime: NaN }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = Infinity → shows generic banned message', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema, {
      reason: Data.Event_ConnectionClosed_CloseReason.BANNED, endTime: Infinity,
    }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with reasonStr → uses reasonStr regardless of endTime', () => {
    connectionClosed(create(Data.Event_ConnectionClosedSchema,
      { reason: Data.Event_ConnectionClosed_CloseReason.BANNED, endTime: 0, reasonStr: 'custom ban reason' }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'custom ban reason');
  });
});

// ----------------------------------------------------------------
// serverIdentification
// ----------------------------------------------------------------
describe('serverIdentification', () => {

  beforeEach(() => {
    ConfigMock.PROTOCOL_VERSION = 14;
    webClient.options = null;
  });

  it('disconnects when protocolVersion mismatches', () => {
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 99, serverOptions: 0 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalled();
    expect(SessionCmds.disconnect).toHaveBeenCalled();
  });

  it('LOGIN reason without salt → calls login with password as separate param', () => {
    webClient.options = { host: 'h', port: '1', userName: 'u', reason: App.WebSocketConnectReason.LOGIN, password: 'secret' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.login).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('LOGIN reason with salt → calls requestPasswordSalt with password as separate param', () => {
    webClient.options = { host: 'h', port: '1', userName: 'u', reason: App.WebSocketConnectReason.LOGIN, password: 'secret' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(1);
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 }));
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('REGISTER reason without salt → calls register with password and null salt', () => {
    webClient.options = {
      host: 'h', port: '1', userName: 'u', email: 'e', country: 'US', realName: 'R',
      reason: App.WebSocketConnectReason.REGISTER, password: 'secret',
    };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.register).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret',
      null
    );
  });

  it('REGISTER reason with salt → calls register with password and generated salt', () => {
    webClient.options = {
      host: 'h', port: '1', userName: 'u', email: 'e', country: 'US', realName: 'R',
      reason: App.WebSocketConnectReason.REGISTER, password: 'secret',
    };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(1);
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 }));
    expect(SessionCmds.register).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret',
      'newSalt'
    );
  });

  it('ACTIVATE_ACCOUNT reason without salt → calls activate with password as separate param', () => {
    webClient.options = {
      host: 'h', port: '1', userName: 'u', token: 'tok',
      reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT, password: 'secret',
    };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.activate).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('ACTIVATE_ACCOUNT reason with salt → calls requestPasswordSalt with password as separate param', () => {
    webClient.options = {
      host: 'h', port: '1', userName: 'u', token: 'tok',
      reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT, password: 'secret',
    };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(1);
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 }));
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('PASSWORD_RESET_REQUEST reason → calls forgotPasswordRequest', () => {
    webClient.options = { host: 'h', port: '1', userName: 'u', reason: App.WebSocketConnectReason.PASSWORD_RESET_REQUEST };
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.forgotPasswordRequest).toHaveBeenCalled();
  });

  it('PASSWORD_RESET_CHALLENGE reason → calls forgotPasswordChallenge', () => {
    webClient.options = { host: 'h', port: '1', userName: 'u', email: 'e', reason: App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE };
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.forgotPasswordChallenge).toHaveBeenCalled();
  });

  it('PASSWORD_RESET reason without salt → calls forgotPasswordReset with newPassword as separate param', () => {
    webClient.options = {
      host: 'h', port: '1', userName: 'u', token: 'tok',
      reason: App.WebSocketConnectReason.PASSWORD_RESET, newPassword: 'newpw',
    };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.forgotPasswordReset).toHaveBeenCalledWith(
      expect.not.objectContaining({ newPassword: expect.anything() }),
      'newpw'
    );
  });

  it('PASSWORD_RESET reason with salt → calls requestPasswordSalt with newPassword as separate param', () => {
    webClient.options = {
      host: 'h', port: '1', userName: 'u', token: 'tok',
      reason: App.WebSocketConnectReason.PASSWORD_RESET, newPassword: 'newpw',
    };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(1);
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 }));
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ newPassword: expect.anything() }),
      undefined,
      'newpw'
    );
  });

  it('unknown reason → updateStatus DISCONNECTED and disconnect', () => {
    webClient.options = { host: 'h', port: '1', reason: 999 as App.WebSocketConnectReason } as Enriched.WebSocketConnectOptions;
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalled();
    expect(SessionCmds.disconnect).toHaveBeenCalled();
  });

  it('resets webClient.options and calls SessionPersistence.updateInfo', () => {
    webClient.options = { host: 'h', port: '1', userName: 'u', reason: App.WebSocketConnectReason.LOGIN };
    serverIdentification(create(Data.Event_ServerIdentificationSchema,
      { serverName: 'myServer', serverVersion: '2.0', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionPersistence.updateInfo).toHaveBeenCalledWith('myServer', '2.0');
    expect(webClient.options).toBeNull();
  });
});
