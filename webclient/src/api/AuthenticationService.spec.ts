vi.mock('@app/websocket', () => ({
  SessionCommands: {
    connect: vi.fn(),
    disconnect: vi.fn(),
  },
}));

vi.mock('../generated/proto/serverinfo_user_pb', async (importOriginal) => {
  const actual = await importOriginal();
  return {
    ...actual,
    ServerInfo_User_UserLevelFlag: {
      IsModerator: 4,
    },
  };
});

import { AuthenticationService } from './AuthenticationService';
import { SessionCommands } from '@app/websocket';
import { App, Data } from '@app/types';
import { create } from '@bufbuild/protobuf';

const baseTransport = { host: 'localhost', port: '4748' };

describe('AuthenticationService', () => {
  describe('login', () => {
    it('calls SessionCommands.connect with LOGIN reason', () => {
      AuthenticationService.login({ ...baseTransport, userName: 'user', password: 'pw' });
      expect(SessionCommands.connect).toHaveBeenCalledWith(
        expect.objectContaining({
          ...baseTransport,
          userName: 'user',
          password: 'pw',
          reason: App.WebSocketConnectReason.LOGIN,
        })
      );
    });
  });

  describe('testConnection', () => {
    it('calls SessionCommands.connect with TEST_CONNECTION reason', () => {
      AuthenticationService.testConnection(baseTransport);
      expect(SessionCommands.connect).toHaveBeenCalledWith(
        expect.objectContaining({ ...baseTransport, reason: App.WebSocketConnectReason.TEST_CONNECTION })
      );
    });
  });

  describe('register', () => {
    it('calls SessionCommands.connect with REGISTER reason', () => {
      AuthenticationService.register({
        ...baseTransport,
        userName: 'user',
        password: 'pw',
        email: 'a@b.com',
        country: 'US',
        realName: 'User',
      });
      expect(SessionCommands.connect).toHaveBeenCalledWith(
        expect.objectContaining({ userName: 'user', reason: App.WebSocketConnectReason.REGISTER })
      );
    });
  });

  describe('activateAccount', () => {
    it('calls SessionCommands.connect with ACTIVATE_ACCOUNT reason', () => {
      AuthenticationService.activateAccount({
        ...baseTransport,
        userName: 'user',
        token: 'tok',
      });
      expect(SessionCommands.connect).toHaveBeenCalledWith(
        expect.objectContaining({ token: 'tok', reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT })
      );
    });
  });

  describe('resetPasswordRequest', () => {
    it('calls SessionCommands.connect with PASSWORD_RESET_REQUEST reason', () => {
      AuthenticationService.resetPasswordRequest({ ...baseTransport, userName: 'user' });
      expect(SessionCommands.connect).toHaveBeenCalledWith(
        expect.objectContaining({ userName: 'user', reason: App.WebSocketConnectReason.PASSWORD_RESET_REQUEST })
      );
    });
  });

  describe('resetPasswordChallenge', () => {
    it('calls SessionCommands.connect with PASSWORD_RESET_CHALLENGE reason', () => {
      AuthenticationService.resetPasswordChallenge({
        ...baseTransport,
        userName: 'user',
        email: 'a@b.com',
      });
      expect(SessionCommands.connect).toHaveBeenCalledWith(
        expect.objectContaining({ email: 'a@b.com', reason: App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE })
      );
    });
  });

  describe('resetPassword', () => {
    it('calls SessionCommands.connect with PASSWORD_RESET reason', () => {
      AuthenticationService.resetPassword({
        ...baseTransport,
        userName: 'user',
        token: 'tok',
        newPassword: 'newpw',
      });
      expect(SessionCommands.connect).toHaveBeenCalledWith(
        expect.objectContaining({ newPassword: 'newpw', reason: App.WebSocketConnectReason.PASSWORD_RESET })
      );
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
      expect(AuthenticationService.isConnected(App.StatusEnum.LOGGED_IN)).toBe(true);
    });

    it('returns false when state is DISCONNECTED', () => {
      expect(AuthenticationService.isConnected(App.StatusEnum.DISCONNECTED)).toBe(false);
    });

    it('returns false when state is CONNECTING', () => {
      expect(AuthenticationService.isConnected(App.StatusEnum.CONNECTING)).toBe(false);
    });

    it('returns false when state is CONNECTED', () => {
      expect(AuthenticationService.isConnected(App.StatusEnum.CONNECTED)).toBe(false);
    });

    it('returns false when state is LOGGING_IN', () => {
      expect(AuthenticationService.isConnected(App.StatusEnum.LOGGING_IN)).toBe(false);
    });
  });

  describe('isModerator', () => {
    it('returns true when userLevel has the IsModerator bit set', () => {
      expect(AuthenticationService.isModerator(create(Data.ServerInfo_UserSchema, { userLevel: 4 }))).toBe(true);
    });

    it('returns true when userLevel has IsModerator and other bits set', () => {
      expect(AuthenticationService.isModerator(create(Data.ServerInfo_UserSchema, { userLevel: 7 }))).toBe(true);
    });

    it('returns false when userLevel does not have the IsModerator bit', () => {
      expect(AuthenticationService.isModerator(create(Data.ServerInfo_UserSchema, { userLevel: 1 }))).toBe(false);
    });

    it('returns false for admin-only userLevel without moderator bit', () => {
      expect(AuthenticationService.isModerator(create(Data.ServerInfo_UserSchema, { userLevel: 8 }))).toBe(false);
    });
  });

  describe('isAdmin', () => {
    it('returns undefined (not yet implemented)', () => {
      expect(AuthenticationService.isAdmin()).toBeUndefined();
    });
  });
});
