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

import { WebSocketConnectReason } from 'types';
import { create } from '@bufbuild/protobuf';
import { Event_ConnectionClosed_CloseReason, Event_ConnectionClosedSchema } from 'generated/proto/event_connection_closed_pb';
import { Event_GameJoinedSchema } from 'generated/proto/event_game_joined_pb';
import { Event_NotifyUserSchema } from 'generated/proto/event_notify_user_pb';
import { Event_ReplayAddedSchema } from 'generated/proto/event_replay_added_pb';
import { Event_ServerCompleteListSchema } from 'generated/proto/event_server_complete_list_pb';
import { Event_ServerMessageSchema } from 'generated/proto/event_server_message_pb';
import { Event_ServerShutdownSchema } from 'generated/proto/event_server_shutdown_pb';
import { Event_UserJoinedSchema } from 'generated/proto/event_user_joined_pb';
import { Event_UserLeftSchema } from 'generated/proto/event_user_left_pb';
import { Event_UserMessageSchema } from 'generated/proto/event_user_message_pb';
import { Event_AddToListSchema } from 'generated/proto/event_add_to_list_pb';
import { Event_RemoveFromListSchema } from 'generated/proto/event_remove_from_list_pb';
import { Event_ListRoomsSchema } from 'generated/proto/event_list_rooms_pb';
import { Event_ServerIdentificationSchema } from 'generated/proto/event_server_identification_pb';

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

beforeEach(() => {
  vi.clearAllMocks();
  (Utils.generateSalt as Mock).mockReturnValue('newSalt');
  (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------
// gameJoined
// ----------------------------------------------------------------
describe('gameJoined', () => {

  it('calls SessionPersistence.gameJoined', () => {
    const data = create(Event_GameJoinedSchema, { playerId: 1 });
    gameJoined(data);
    expect(SessionPersistence.gameJoined).toHaveBeenCalledWith(data);
  });
});

// ----------------------------------------------------------------
// notifyUser
// ----------------------------------------------------------------
describe('notifyUser', () => {

  it('calls SessionPersistence.notifyUser', () => {
    const data = create(Event_NotifyUserSchema, { warningReason: 'yo' });
    notifyUser(data);
    expect(SessionPersistence.notifyUser).toHaveBeenCalledWith(data);
  });
});

// ----------------------------------------------------------------
// replayAdded
// ----------------------------------------------------------------
describe('replayAdded', () => {

  it('calls SessionPersistence.replayAdded with matchInfo', () => {
    const data = create(Event_ReplayAddedSchema);
    data.matchInfo = { gameId: 42 } as any;
    replayAdded(data);
    expect(SessionPersistence.replayAdded).toHaveBeenCalledWith(data.matchInfo);
  });
});

// ----------------------------------------------------------------
// serverCompleteList
// ----------------------------------------------------------------
describe('serverCompleteList', () => {

  it('calls SessionPersistence.updateUsers and RoomPersistence.updateRooms', () => {
    const data = create(Event_ServerCompleteListSchema, { userList: [], roomList: [] });
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
    serverMessage(create(Event_ServerMessageSchema, { message: 'hello server' }));
    expect(SessionPersistence.serverMessage).toHaveBeenCalledWith('hello server');
  });
});

// ----------------------------------------------------------------
// serverShutdown
// ----------------------------------------------------------------
describe('serverShutdown', () => {

  it('calls SessionPersistence.serverShutdown', () => {
    const payload = create(Event_ServerShutdownSchema, { reason: 'maintenance' });
    serverShutdown(payload);
    expect(SessionPersistence.serverShutdown).toHaveBeenCalledWith(payload);
  });
});

// ----------------------------------------------------------------
// userJoined
// ----------------------------------------------------------------
describe('userJoined', () => {

  it('calls SessionPersistence.userJoined with userInfo', () => {
    const data = create(Event_UserJoinedSchema);
    data.userInfo = { name: 'alice' } as any;
    userJoined(data);
    expect(SessionPersistence.userJoined).toHaveBeenCalledWith(data.userInfo);
  });
});

// ----------------------------------------------------------------
// userLeft
// ----------------------------------------------------------------
describe('userLeft', () => {

  it('calls SessionPersistence.userLeft with name', () => {
    userLeft(create(Event_UserLeftSchema, { name: 'bob' }));
    expect(SessionPersistence.userLeft).toHaveBeenCalledWith('bob');
  });
});

// ----------------------------------------------------------------
// userMessage
// ----------------------------------------------------------------
describe('userMessage', () => {

  it('calls SessionPersistence.userMessage', () => {
    const payload = create(Event_UserMessageSchema, { senderName: 'alice', message: 'hi' });
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
    const data = create(Event_AddToListSchema, { listName: 'buddy' });
    data.userInfo = { name: 'alice' } as any;
    addToList(data);
    expect(SessionPersistence.addToBuddyList).toHaveBeenCalledWith(data.userInfo);
  });

  it('ignore list → addToIgnoreList', () => {
    const data = create(Event_AddToListSchema, { listName: 'ignore' });
    data.userInfo = { name: 'bob' } as any;
    addToList(data);
    expect(SessionPersistence.addToIgnoreList).toHaveBeenCalledWith(data.userInfo);
  });

  it('unknown list → console.log', () => {
    addToList(create(Event_AddToListSchema, { listName: 'unknown' }));
    expect(logSpy).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// removeFromList
// ----------------------------------------------------------------
describe('removeFromList', () => {

  it('buddy list → removeFromBuddyList', () => {
    removeFromList(create(Event_RemoveFromListSchema, { listName: 'buddy', userName: 'alice' }));
    expect(SessionPersistence.removeFromBuddyList).toHaveBeenCalledWith('alice');
  });

  it('ignore list → removeFromIgnoreList', () => {
    removeFromList(create(Event_RemoveFromListSchema, { listName: 'ignore', userName: 'bob' }));
    expect(SessionPersistence.removeFromIgnoreList).toHaveBeenCalledWith('bob');
  });

  it('unknown list → console.log', () => {
    const logSpy = vi.spyOn(console, 'log').mockImplementation(() => {});
    removeFromList(create(Event_RemoveFromListSchema, { listName: 'other', userName: 'x' }));
    expect(logSpy).toHaveBeenCalled();
    logSpy.mockRestore();
  });
});

// ----------------------------------------------------------------
// listRooms
// ----------------------------------------------------------------
describe('listRooms', () => {

  it('calls RoomPersistence.updateRooms', () => {
    listRooms(create(Event_ListRoomsSchema, { roomList: [] }));
    expect(RoomPersistence.updateRooms).toHaveBeenCalledWith([]);
  });

  it('does not call joinRoom when autojoinrooms is false', () => {
    (Config as any).CLIENT_OPTIONS = { autojoinrooms: false };
    listRooms(create(Event_ListRoomsSchema, { roomList: [{ autoJoin: true, roomId: 1 }] as any[] }));
    expect(SessionCmds.joinRoom).not.toHaveBeenCalled();
  });

  it('calls joinRoom for autoJoin rooms when autojoinrooms is true', () => {
    (Config as any).CLIENT_OPTIONS = { autojoinrooms: true };
    listRooms(create(Event_ListRoomsSchema, { roomList: [{ autoJoin: true, roomId: 2 }, { autoJoin: false, roomId: 3 }] as any[] }));
    expect(SessionCmds.joinRoom).toHaveBeenCalledTimes(1);
    expect(SessionCmds.joinRoom).toHaveBeenCalledWith(2);
  });
});

// ----------------------------------------------------------------
// connectionClosed
// ----------------------------------------------------------------
describe('connectionClosed', () => {

  it('uses reasonStr when provided', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: 0, reasonStr: 'custom' }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'custom');
  });

  it('USER_LIMIT_REACHED → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.USER_LIMIT_REACHED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(
      expect.anything(),
      expect.stringContaining('maximum user capacity')
    );
  });

  it('TOO_MANY_CONNECTIONS → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.TOO_MANY_CONNECTIONS }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('too many concurrent'));
  });

  it('BANNED → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('DEMOTED → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.DEMOTED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('demoted'));
  });

  it('SERVER_SHUTDOWN → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.SERVER_SHUTDOWN }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('shutdown'));
  });

  it('USERNAMEINVALID → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.USERNAMEINVALID }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('username'));
  });

  it('LOGGEDINELSEWERE → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.LOGGEDINELSEWERE }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('logged out'));
  });

  it('OTHER → "Unknown reason"', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.OTHER }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'Unknown reason');
  });

  it('BANNED with valid positive endTime → shows formatted date', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: 1700000000 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(
      expect.anything(),
      expect.stringContaining('You are banned until')
    );
  });

  it('BANNED with endTime = 0 → shows generic banned message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: 0 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = -1 → shows generic banned message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: -1 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = NaN → shows generic banned message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: NaN }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = Infinity → shows generic banned message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: Infinity }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with reasonStr → uses reasonStr regardless of endTime', () => {
    connectionClosed(create(Event_ConnectionClosedSchema,
      { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: 0, reasonStr: 'custom ban reason' }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'custom ban reason');
  });
});

// ----------------------------------------------------------------
// serverIdentification
// ----------------------------------------------------------------
describe('serverIdentification', () => {

  beforeEach(() => {
    (Config as any).PROTOCOL_VERSION = 14;
    (webClient as any).options = {};
  });

  it('disconnects when protocolVersion mismatches', () => {
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 99, serverOptions: 0 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalled();
    expect(SessionCmds.disconnect).toHaveBeenCalled();
  });

  it('LOGIN reason without salt → calls login with password as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.LOGIN, password: 'secret' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.login).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('LOGIN reason with salt → calls requestPasswordSalt with password as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.LOGIN, password: 'secret' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(1);
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 }));
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('REGISTER reason without salt → calls register with password and null salt', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.REGISTER, password: 'secret' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.register).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret',
      null
    );
  });

  it('REGISTER reason with salt → calls register with password and generated salt', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.REGISTER, password: 'secret' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(1);
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 }));
    expect(SessionCmds.register).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret',
      'newSalt'
    );
  });

  it('ACTIVATE_ACCOUNT reason without salt → calls activate with password as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.ACTIVATE_ACCOUNT, password: 'secret' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.activate).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('ACTIVATE_ACCOUNT reason with salt → calls requestPasswordSalt with password as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.ACTIVATE_ACCOUNT, password: 'secret' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(1);
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 }));
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('PASSWORD_RESET_REQUEST reason → calls forgotPasswordRequest', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.PASSWORD_RESET_REQUEST };
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.forgotPasswordRequest).toHaveBeenCalled();
  });

  it('PASSWORD_RESET_CHALLENGE reason → calls forgotPasswordChallenge', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.PASSWORD_RESET_CHALLENGE };
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.forgotPasswordChallenge).toHaveBeenCalled();
  });

  it('PASSWORD_RESET reason without salt → calls forgotPasswordReset with newPassword as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.PASSWORD_RESET, newPassword: 'newpw' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(0);
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.forgotPasswordReset).toHaveBeenCalledWith(
      expect.not.objectContaining({ newPassword: expect.anything() }),
      'newpw'
    );
  });

  it('PASSWORD_RESET reason with salt → calls requestPasswordSalt with newPassword as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.PASSWORD_RESET, newPassword: 'newpw' };
    (Utils.passwordSaltSupported as Mock).mockReturnValue(1);
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 }));
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ newPassword: expect.anything() }),
      undefined,
      'newpw'
    );
  });

  it('unknown reason → updateStatus DISCONNECTED and disconnect', () => {
    (webClient as any).options = { reason: 999 };
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalled();
    expect(SessionCmds.disconnect).toHaveBeenCalled();
  });

  it('updates webClient.options to empty and calls SessionPersistence.updateInfo', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.LOGIN };
    serverIdentification(create(Event_ServerIdentificationSchema,
      { serverName: 'myServer', serverVersion: '2.0', protocolVersion: 14, serverOptions: 0 }));
    expect(SessionPersistence.updateInfo).toHaveBeenCalledWith('myServer', '2.0');
    expect((webClient as any).options).toEqual({});
  });
});
