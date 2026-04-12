// Tests for simple session events that delegate 1:1 to SessionPersistence
// or RoomPersistence with minimal logic.

jest.mock('../../persistence', () => ({
  SessionPersistence: {
    gameJoined: jest.fn(),
    notifyUser: jest.fn(),
    replayAdded: jest.fn(),
    serverMessage: jest.fn(),
    serverShutdown: jest.fn(),
    updateUsers: jest.fn(),
    updateInfo: jest.fn(),
    userJoined: jest.fn(),
    userLeft: jest.fn(),
    userMessage: jest.fn(),
    addToBuddyList: jest.fn(),
    addToIgnoreList: jest.fn(),
    removeFromBuddyList: jest.fn(),
    removeFromIgnoreList: jest.fn(),
    playerPropertiesChanged: jest.fn(),
  },
  RoomPersistence: {
    updateRooms: jest.fn(),
  },
}));

jest.mock('../../WebClient', () => ({
  __esModule: true,
  default: {
    clientOptions: { autojoinrooms: false },
    options: {},
    protocolVersion: 14,
  },
}));

jest.mock('../../commands/session', () => ({
  joinRoom: jest.fn(),
  updateStatus: jest.fn(),
  disconnect: jest.fn(),
  login: jest.fn(),
  register: jest.fn(),
  activate: jest.fn(),
  requestPasswordSalt: jest.fn(),
  forgotPasswordRequest: jest.fn(),
  forgotPasswordChallenge: jest.fn(),
  forgotPasswordReset: jest.fn(),
}));

jest.mock('../../utils', () => ({
  generateSalt: jest.fn().mockReturnValue('newSalt'),
  passwordSaltSupported: jest.fn().mockReturnValue(0),
}));

jest.mock('../../services/ProtoController', () => ({
  ProtoController: {
    root: {
      Event_ConnectionClosed: {
        CloseReason: {
          USER_LIMIT_REACHED: 0,
          TOO_MANY_CONNECTIONS: 1,
          BANNED: 2,
          DEMOTED: 3,
          SERVER_SHUTDOWN: 4,
          USERNAMEINVALID: 5,
          LOGGEDINELSEWERE: 6,
          OTHER: 7,
        },
      },
    },
  },
}));

import { WebSocketConnectReason } from 'types';

import { SessionPersistence, RoomPersistence } from '../../persistence';
import webClient from '../../WebClient';
import * as SessionCmds from '../../commands/session';
import * as Utils from '../../utils';

beforeEach(() => {
  jest.clearAllMocks();
  (Utils.generateSalt as jest.Mock).mockReturnValue('newSalt');
  (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------
// gameJoined
// ----------------------------------------------------------------
describe('gameJoined', () => {
  const { gameJoined } = jest.requireActual('./gameJoined');

  it('calls SessionPersistence.gameJoined', () => {
    const data = { gameId: 1 } as any;
    gameJoined(data);
    expect(SessionPersistence.gameJoined).toHaveBeenCalledWith(data);
  });
});

// ----------------------------------------------------------------
// notifyUser
// ----------------------------------------------------------------
describe('notifyUser', () => {
  const { notifyUser } = jest.requireActual('./notifyUser');

  it('calls SessionPersistence.notifyUser', () => {
    const data = { message: 'yo' } as any;
    notifyUser(data);
    expect(SessionPersistence.notifyUser).toHaveBeenCalledWith(data);
  });
});

// ----------------------------------------------------------------
// replayAdded
// ----------------------------------------------------------------
describe('replayAdded', () => {
  const { replayAdded } = jest.requireActual('./replayAdded');

  it('calls SessionPersistence.replayAdded with matchInfo', () => {
    replayAdded({ matchInfo: { id: 42 } } as any);
    expect(SessionPersistence.replayAdded).toHaveBeenCalledWith({ id: 42 });
  });
});

// ----------------------------------------------------------------
// serverCompleteList
// ----------------------------------------------------------------
describe('serverCompleteList', () => {
  const { serverCompleteList } = jest.requireActual('./serverCompleteList');

  it('calls SessionPersistence.updateUsers and RoomPersistence.updateRooms', () => {
    serverCompleteList({ userList: ['u'], roomList: ['r'] } as any);
    expect(SessionPersistence.updateUsers).toHaveBeenCalledWith(['u']);
    expect(RoomPersistence.updateRooms).toHaveBeenCalledWith(['r']);
  });
});

// ----------------------------------------------------------------
// serverMessage
// ----------------------------------------------------------------
describe('serverMessage', () => {
  const { serverMessage } = jest.requireActual('./serverMessage');

  it('calls SessionPersistence.serverMessage with message', () => {
    serverMessage({ message: 'hello server' });
    expect(SessionPersistence.serverMessage).toHaveBeenCalledWith('hello server');
  });
});

// ----------------------------------------------------------------
// serverShutdown
// ----------------------------------------------------------------
describe('serverShutdown', () => {
  const { serverShutdown } = jest.requireActual('./serverShutdown');

  it('calls SessionPersistence.serverShutdown', () => {
    const payload = { reason: 'maintenance' } as any;
    serverShutdown(payload);
    expect(SessionPersistence.serverShutdown).toHaveBeenCalledWith(payload);
  });
});

// ----------------------------------------------------------------
// userJoined
// ----------------------------------------------------------------
describe('userJoined', () => {
  const { userJoined } = jest.requireActual('./userJoined');

  it('calls SessionPersistence.userJoined with userInfo', () => {
    userJoined({ userInfo: { name: 'alice' } } as any);
    expect(SessionPersistence.userJoined).toHaveBeenCalledWith({ name: 'alice' });
  });
});

// ----------------------------------------------------------------
// userLeft
// ----------------------------------------------------------------
describe('userLeft', () => {
  const { userLeft } = jest.requireActual('./userLeft');

  it('calls SessionPersistence.userLeft with name', () => {
    userLeft({ name: 'bob' });
    expect(SessionPersistence.userLeft).toHaveBeenCalledWith('bob');
  });
});

// ----------------------------------------------------------------
// userMessage
// ----------------------------------------------------------------
describe('userMessage', () => {
  const { userMessage } = jest.requireActual('./userMessage');

  it('calls SessionPersistence.userMessage', () => {
    const payload = { userName: 'alice', message: 'hi' } as any;
    userMessage(payload);
    expect(SessionPersistence.userMessage).toHaveBeenCalledWith(payload);
  });
});

// ----------------------------------------------------------------
// addToList
// ----------------------------------------------------------------
describe('addToList', () => {
  const { addToList } = jest.requireActual('./addToList');
  const logSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
  afterAll(() => logSpy.mockRestore());

  it('buddy list → addToBuddyList', () => {
    addToList({ listName: 'buddy', userInfo: { name: 'alice' } } as any);
    expect(SessionPersistence.addToBuddyList).toHaveBeenCalledWith({ name: 'alice' });
  });

  it('ignore list → addToIgnoreList', () => {
    addToList({ listName: 'ignore', userInfo: { name: 'bob' } } as any);
    expect(SessionPersistence.addToIgnoreList).toHaveBeenCalledWith({ name: 'bob' });
  });

  it('unknown list → console.log', () => {
    addToList({ listName: 'unknown', userInfo: {} } as any);
    expect(logSpy).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// removeFromList
// ----------------------------------------------------------------
describe('removeFromList', () => {
  const { removeFromList } = jest.requireActual('./removeFromList');

  it('buddy list → removeFromBuddyList', () => {
    removeFromList({ listName: 'buddy', userName: 'alice' } as any);
    expect(SessionPersistence.removeFromBuddyList).toHaveBeenCalledWith('alice');
  });

  it('ignore list → removeFromIgnoreList', () => {
    removeFromList({ listName: 'ignore', userName: 'bob' } as any);
    expect(SessionPersistence.removeFromIgnoreList).toHaveBeenCalledWith('bob');
  });

  it('unknown list → console.log', () => {
    const logSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
    removeFromList({ listName: 'other', userName: 'x' } as any);
    expect(logSpy).toHaveBeenCalled();
    logSpy.mockRestore();
  });
});

// ----------------------------------------------------------------
// listRooms
// ----------------------------------------------------------------
describe('listRooms', () => {
  const { listRooms } = jest.requireActual('./listRooms');

  it('calls RoomPersistence.updateRooms', () => {
    listRooms({ roomList: [] });
    expect(RoomPersistence.updateRooms).toHaveBeenCalledWith([]);
  });

  it('does not call joinRoom when autojoinrooms is false', () => {
    (webClient as any).clientOptions = { autojoinrooms: false };
    listRooms({ roomList: [{ autoJoin: true, roomId: 1 }] } as any);
    expect(SessionCmds.joinRoom).not.toHaveBeenCalled();
  });

  it('calls joinRoom for autoJoin rooms when autojoinrooms is true', () => {
    (webClient as any).clientOptions = { autojoinrooms: true };
    listRooms({ roomList: [{ autoJoin: true, roomId: 2 }, { autoJoin: false, roomId: 3 }] } as any);
    expect(SessionCmds.joinRoom).toHaveBeenCalledTimes(1);
    expect(SessionCmds.joinRoom).toHaveBeenCalledWith(2);
  });
});

// ----------------------------------------------------------------
// connectionClosed
// ----------------------------------------------------------------
describe('connectionClosed', () => {
  const { connectionClosed } = jest.requireActual('./connectionClosed');

  it('uses reasonStr when provided', () => {
    connectionClosed({ reason: 0, reasonStr: 'custom' } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'custom');
  });

  it('USER_LIMIT_REACHED → specific message', () => {
    connectionClosed({ reason: 0 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(
      expect.anything(),
      expect.stringContaining('maximum user capacity')
    );
  });

  it('TOO_MANY_CONNECTIONS → specific message', () => {
    connectionClosed({ reason: 1 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('too many concurrent'));
  });

  it('BANNED → specific message', () => {
    connectionClosed({ reason: 2 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('banned'));
  });

  it('DEMOTED → specific message', () => {
    connectionClosed({ reason: 3 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('demoted'));
  });

  it('SERVER_SHUTDOWN → specific message', () => {
    connectionClosed({ reason: 4 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('shutdown'));
  });

  it('USERNAMEINVALID → specific message', () => {
    connectionClosed({ reason: 5 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('username'));
  });

  it('LOGGEDINELSEWERE → specific message', () => {
    connectionClosed({ reason: 6 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('logged out'));
  });

  it('OTHER → "Unknown reason"', () => {
    connectionClosed({ reason: 7 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'Unknown reason');
  });

  it('BANNED with valid positive endTime → shows formatted date', () => {
    connectionClosed({ reason: 2, endTime: 1700000000 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(
      expect.anything(),
      expect.stringContaining('You are banned until')
    );
  });

  it('BANNED with endTime = 0 → shows generic banned message', () => {
    connectionClosed({ reason: 2, endTime: 0 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = -1 → shows generic banned message', () => {
    connectionClosed({ reason: 2, endTime: -1 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = NaN → shows generic banned message', () => {
    connectionClosed({ reason: 2, endTime: NaN } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = Infinity → shows generic banned message', () => {
    connectionClosed({ reason: 2, endTime: Infinity } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with reasonStr → uses reasonStr regardless of endTime', () => {
    connectionClosed({ reason: 2, endTime: 0, reasonStr: 'custom ban reason' } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'custom ban reason');
  });
});

// ----------------------------------------------------------------
// serverIdentification
// ----------------------------------------------------------------
describe('serverIdentification', () => {
  const { serverIdentification } = jest.requireActual('./serverIdentification');

  beforeEach(() => {
    (webClient as any).protocolVersion = 14;
    (webClient as any).options = {};
  });

  it('disconnects when protocolVersion mismatches', () => {
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 99, serverOptions: 0 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalled();
    expect(SessionCmds.disconnect).toHaveBeenCalled();
  });

  it('LOGIN reason without salt → calls login with password as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.LOGIN, password: 'secret' };
    (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(0);
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 } as any);
    expect(SessionCmds.login).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('LOGIN reason with salt → calls requestPasswordSalt with password as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.LOGIN, password: 'secret' };
    (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(1);
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 } as any);
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('REGISTER reason without salt → calls register with password and null salt', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.REGISTER, password: 'secret' };
    (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(0);
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 } as any);
    expect(SessionCmds.register).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret',
      null
    );
  });

  it('REGISTER reason with salt → calls register with password and generated salt', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.REGISTER, password: 'secret' };
    (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(1);
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 } as any);
    expect(SessionCmds.register).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret',
      'newSalt'
    );
  });

  it('ACTIVATE_ACCOUNT reason without salt → calls activate with password as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.ACTIVATE_ACCOUNT, password: 'secret' };
    (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(0);
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 } as any);
    expect(SessionCmds.activate).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('ACTIVATE_ACCOUNT reason with salt → calls requestPasswordSalt with password as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.ACTIVATE_ACCOUNT, password: 'secret' };
    (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(1);
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 } as any);
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() }),
      'secret'
    );
  });

  it('PASSWORD_RESET_REQUEST reason → calls forgotPasswordRequest', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.PASSWORD_RESET_REQUEST };
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 } as any);
    expect(SessionCmds.forgotPasswordRequest).toHaveBeenCalled();
  });

  it('PASSWORD_RESET_CHALLENGE reason → calls forgotPasswordChallenge', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.PASSWORD_RESET_CHALLENGE };
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 } as any);
    expect(SessionCmds.forgotPasswordChallenge).toHaveBeenCalled();
  });

  it('PASSWORD_RESET reason without salt → calls forgotPasswordReset with newPassword as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.PASSWORD_RESET, newPassword: 'newpw' };
    (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(0);
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 } as any);
    expect(SessionCmds.forgotPasswordReset).toHaveBeenCalledWith(
      expect.not.objectContaining({ newPassword: expect.anything() }),
      'newpw'
    );
  });

  it('PASSWORD_RESET reason with salt → calls requestPasswordSalt with newPassword as separate param', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.PASSWORD_RESET, newPassword: 'newpw' };
    (Utils.passwordSaltSupported as jest.Mock).mockReturnValue(1);
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 1 } as any);
    expect(SessionCmds.requestPasswordSalt).toHaveBeenCalledWith(
      expect.not.objectContaining({ newPassword: expect.anything() }),
      undefined,
      'newpw'
    );
  });

  it('unknown reason → updateStatus DISCONNECTED and disconnect', () => {
    (webClient as any).options = { reason: 999 };
    serverIdentification({ serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 } as any);
    expect(SessionCmds.updateStatus).toHaveBeenCalled();
    expect(SessionCmds.disconnect).toHaveBeenCalled();
  });

  it('updates webClient.options to empty and calls SessionPersistence.updateInfo', () => {
    (webClient as any).options = { reason: WebSocketConnectReason.LOGIN };
    serverIdentification({ serverName: 'myServer', serverVersion: '2.0', protocolVersion: 14, serverOptions: 0 } as any);
    expect(SessionPersistence.updateInfo).toHaveBeenCalledWith('myServer', '2.0');
    expect((webClient as any).options).toEqual({});
  });
});
