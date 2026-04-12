vi.mock('websocket', () => ({
  SessionCommands: {
    connect: vi.fn(),
    disconnect: vi.fn(),
  },
  webClient: {
    connectionAttemptMade: false,
  },
}));

vi.mock('websocket/services/ProtoController', () => ({
  ProtoController: {
    root: {
      ServerInfo_User: {
        UserLevelFlag: {
          IsModerator: 4,
        },
      },
    },
  },
}));

import { AuthenticationService } from './AuthenticationService';
import { SessionCommands, webClient } from 'websocket';
import { StatusEnum, WebSocketConnectOptions, WebSocketConnectReason } from 'types';

const testOptions: WebSocketConnectOptions = { host: 'localhost', port: '4748', userName: 'user', password: 'pw' };

beforeEach(() => vi.clearAllMocks());

describe('AuthenticationService', () => {
  describe('login', () => {
    it('calls SessionCommands.connect with LOGIN reason', () => {
      AuthenticationService.login(testOptions);
      expect(SessionCommands.connect).toHaveBeenCalledWith(testOptions, WebSocketConnectReason.LOGIN);
    });
  });

  describe('testConnection', () => {
    it('calls SessionCommands.connect with TEST_CONNECTION reason', () => {
      AuthenticationService.testConnection(testOptions);
      expect(SessionCommands.connect).toHaveBeenCalledWith(testOptions, WebSocketConnectReason.TEST_CONNECTION);
    });
  });

  describe('register', () => {
    it('calls SessionCommands.connect with REGISTER reason', () => {
      AuthenticationService.register(testOptions);
      expect(SessionCommands.connect).toHaveBeenCalledWith(testOptions, WebSocketConnectReason.REGISTER);
    });
  });

  describe('activateAccount', () => {
    it('calls SessionCommands.connect with ACTIVATE_ACCOUNT reason', () => {
      AuthenticationService.activateAccount(testOptions);
      expect(SessionCommands.connect).toHaveBeenCalledWith(testOptions, WebSocketConnectReason.ACTIVATE_ACCOUNT);
    });
  });

  describe('resetPasswordRequest', () => {
    it('calls SessionCommands.connect with PASSWORD_RESET_REQUEST reason', () => {
      AuthenticationService.resetPasswordRequest(testOptions);
      expect(SessionCommands.connect).toHaveBeenCalledWith(testOptions, WebSocketConnectReason.PASSWORD_RESET_REQUEST);
    });
  });

  describe('resetPasswordChallenge', () => {
    it('calls SessionCommands.connect with PASSWORD_RESET_CHALLENGE reason', () => {
      AuthenticationService.resetPasswordChallenge(testOptions);
      expect(SessionCommands.connect).toHaveBeenCalledWith(testOptions, WebSocketConnectReason.PASSWORD_RESET_CHALLENGE);
    });
  });

  describe('resetPassword', () => {
    it('calls SessionCommands.connect with PASSWORD_RESET reason', () => {
      AuthenticationService.resetPassword(testOptions);
      expect(SessionCommands.connect).toHaveBeenCalledWith(testOptions, WebSocketConnectReason.PASSWORD_RESET);
    });
  });

  describe('disconnect', () => {
    it('delegates to SessionCommands.disconnect', () => {
      AuthenticationService.disconnect();
      expect(SessionCommands.disconnect).toHaveBeenCalled();
    });
  });

  describe('isConnected', () => {
    it('returns true when state is LOGGED_IN', () => {
      expect(AuthenticationService.isConnected(StatusEnum.LOGGED_IN)).toBe(true);
    });

    it('returns false when state is DISCONNECTED', () => {
      expect(AuthenticationService.isConnected(StatusEnum.DISCONNECTED)).toBe(false);
    });

    it('returns false when state is CONNECTING', () => {
      expect(AuthenticationService.isConnected(StatusEnum.CONNECTING)).toBe(false);
    });

    it('returns false when state is CONNECTED', () => {
      expect(AuthenticationService.isConnected(StatusEnum.CONNECTED)).toBe(false);
    });

    it('returns false when state is LOGGING_IN', () => {
      expect(AuthenticationService.isConnected(StatusEnum.LOGGING_IN)).toBe(false);
    });
  });

  describe('isModerator', () => {
    it('returns true when userLevel has the IsModerator bit set', () => {
      expect(AuthenticationService.isModerator({ userLevel: 4 } as any)).toBe(true);
    });

    it('returns true when userLevel has IsModerator and other bits set', () => {
      expect(AuthenticationService.isModerator({ userLevel: 7 } as any)).toBe(true);
    });

    it('returns false when userLevel does not have the IsModerator bit', () => {
      expect(AuthenticationService.isModerator({ userLevel: 1 } as any)).toBe(false);
    });

    it('returns false for admin-only userLevel without moderator bit', () => {
      expect(AuthenticationService.isModerator({ userLevel: 8 } as any)).toBe(false);
    });
  });

  describe('isAdmin', () => {
    it('returns undefined (not yet implemented)', () => {
      expect(AuthenticationService.isAdmin()).toBeUndefined();
    });
  });

  describe('connectionAttemptMade', () => {
    it('returns webClient.connectionAttemptMade when false', () => {
      (webClient as any).connectionAttemptMade = false;
      expect(AuthenticationService.connectionAttemptMade()).toBe(false);
    });

    it('returns webClient.connectionAttemptMade when true', () => {
      (webClient as any).connectionAttemptMade = true;
      expect(AuthenticationService.connectionAttemptMade()).toBe(true);
    });
  });
});
