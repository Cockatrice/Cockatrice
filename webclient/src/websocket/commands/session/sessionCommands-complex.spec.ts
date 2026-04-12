// Tests for complex session commands that call webClient directly
// or have multiple branching callbacks.

jest.mock('../../services/BackendService', () => ({
  BackendService: {
    sendSessionCommand: jest.fn(),
  },
}));

jest.mock('../../persistence', () => {
  const { makeSessionPersistenceMock } = require('../../__mocks__/sessionCommandMocks');
  return {
    SessionPersistence: makeSessionPersistenceMock(),
    RoomPersistence: {},
  };
});

jest.mock('../../WebClient', () => {
  const { makeWebClientMock } = require('../../__mocks__/sessionCommandMocks');
  return { __esModule: true, default: makeWebClientMock() };
});

jest.mock('../../services/ProtoController', () => {
  const { makeProtoControllerRootMock } = require('../../__mocks__/sessionCommandMocks');
  return { ProtoController: { root: makeProtoControllerRootMock() } };
});

jest.mock('../../utils', () => {
  const { makeUtilsMock } = require('../../__mocks__/sessionCommandMocks');
  return makeUtilsMock();
});

// Intercept all re-exported commands to avoid recursive real invocations
jest.mock('./', () => {
  const { makeSessionBarrelMock } = require('../../__mocks__/sessionCommandMocks');
  return makeSessionBarrelMock();
});

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';
import webClient from '../../WebClient';
import * as SessionIndexMocks from './';
import { StatusEnum, WebSocketConnectReason } from 'types';
import { hashPassword, generateSalt, passwordSaltSupported } from '../../utils';

const { getLastSendOpts, invokeOnSuccess, invokeResponseCode, invokeOnError } = makeCallbackHelpers(
  BackendService.sendSessionCommand as jest.Mock
);

beforeEach(() => {
  jest.clearAllMocks();
  (hashPassword as jest.Mock).mockReturnValue('hashed_pw');
  (generateSalt as jest.Mock).mockReturnValue('randSalt');
  (passwordSaltSupported as jest.Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------
// connect.ts
// ----------------------------------------------------------------
describe('connect', () => {
  const { connect } = jest.requireActual('./connect');

  it('calls updateStatus CONNECTING for LOGIN reason', () => {
    connect({ host: 'h', port: 1 } as any, WebSocketConnectReason.LOGIN);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, 'Connecting...');
    expect(webClient.connect).toHaveBeenCalled();
  });

  it('calls updateStatus CONNECTING for REGISTER reason', () => {
    connect({ host: 'h', port: 1 } as any, WebSocketConnectReason.REGISTER);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls updateStatus CONNECTING for ACTIVATE_ACCOUNT reason', () => {
    connect({ host: 'h', port: 1 } as any, WebSocketConnectReason.ACTIVATE_ACCOUNT);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls updateStatus CONNECTING for PASSWORD_RESET_REQUEST reason', () => {
    connect({ host: 'h', port: 1 } as any, WebSocketConnectReason.PASSWORD_RESET_REQUEST);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls updateStatus CONNECTING for PASSWORD_RESET_CHALLENGE reason', () => {
    connect({ host: 'h', port: 1 } as any, WebSocketConnectReason.PASSWORD_RESET_CHALLENGE);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls updateStatus CONNECTING for PASSWORD_RESET reason', () => {
    connect({ host: 'h', port: 1 } as any, WebSocketConnectReason.PASSWORD_RESET);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls testConnect for TEST_CONNECTION reason', () => {
    connect({ host: 'h', port: 1 } as any, WebSocketConnectReason.TEST_CONNECTION);
    expect(webClient.testConnect).toHaveBeenCalled();
    expect(webClient.connect).not.toHaveBeenCalled();
  });

  it('calls updateStatus DISCONNECTED for unknown reason', () => {
    connect({ host: 'h', port: 1 } as any, 999 as WebSocketConnectReason);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, expect.stringContaining('Unknown'));
  });
});

// ----------------------------------------------------------------
// updateStatus.ts
// ----------------------------------------------------------------
describe('updateStatus', () => {
  const { updateStatus } = jest.requireActual('./updateStatus');

  it('calls SessionPersistence.updateStatus and webClient.updateStatus', () => {
    updateStatus(StatusEnum.CONNECTED, 'OK');
    expect(SessionPersistence.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTED, 'OK');
    expect(webClient.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTED);
  });
});

// ----------------------------------------------------------------
// login.ts
// ----------------------------------------------------------------
describe('login', () => {
  const { login } = jest.requireActual('./login');

  it('sends Command_Login with plain password when no salt', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_Login',
      expect.objectContaining({ userName: 'alice', password: 'pw' }),
      expect.any(Object)
    );
  });

  it('sends Command_Login with hashedPassword when salt is given', () => {
    login({ userName: 'alice', password: 'pw' } as any, 'salt');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_Login',
      expect.objectContaining({ hashedPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('uses options.hashedPassword if provided', () => {
    login({ userName: 'alice', password: 'pw', hashedPassword: 'pre_hashed' } as any, 'salt');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_Login',
      expect.objectContaining({ hashedPassword: 'pre_hashed' }),
      expect.any(Object)
    );
  });

  it('onSuccess dispatches buddy/ignore/user and calls listUsers/listRooms', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0, '.Response_Login.ext': loginResp });
    expect(SessionPersistence.updateBuddyList).toHaveBeenCalledWith([]);
    expect(SessionPersistence.updateIgnoreList).toHaveBeenCalledWith([]);
    expect(SessionPersistence.updateUser).toHaveBeenCalledWith({ name: 'alice' });
    expect(SessionPersistence.loginSuccessful).toHaveBeenCalled();
    expect(SessionIndexMocks.listUsers).toHaveBeenCalled();
    expect(SessionIndexMocks.listRooms).toHaveBeenCalled();
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.LOGGED_IN, 'Logged in.');
  });

  it('onSuccess does NOT pass plaintext password to loginSuccessful', () => {
    login({ userName: 'alice', password: 'secret' } as any);
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0, '.Response_Login.ext': loginResp });
    const calledWith = (SessionPersistence.loginSuccessful as jest.Mock).mock.calls[0][0];
    expect(calledWith).not.toHaveProperty('password');
  });

  it('onSuccess passes hashedPassword to loginSuccessful when salt is used', () => {
    login({ userName: 'alice', password: 'pw' } as any, 'salt');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0, '.Response_Login.ext': loginResp });
    const calledWith = (SessionPersistence.loginSuccessful as jest.Mock).mock.calls[0][0];
    expect(calledWith).toHaveProperty('hashedPassword', 'hashed_pw');
  });

  it('onResponseCode RespClientUpdateRequired calls onLoginError', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(1);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onResponseCode RespWrongPassword', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(2);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespUsernameInvalid', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(3);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespWouldOverwriteOldSession', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(4);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespUserIsBanned', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(5);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespRegistrationRequired', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(6);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespClientIdRequired', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(7);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespContextError', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(8);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespAccountNotActivated calls accountAwaitingActivation', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(9);
    expect(SessionPersistence.accountAwaitingActivation).toHaveBeenCalled();
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onError calls onLoginError with unknown error message', () => {
    login({ userName: 'alice', password: 'pw' } as any);
    invokeOnError(999);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// register.ts
// ----------------------------------------------------------------
describe('register', () => {
  const { register } = jest.requireActual('./register');

  it('sends Command_Register with plain password when no salt', () => {
    register({ userName: 'alice', password: 'pw', email: 'a@b.com', country: 'US', realName: 'Al' } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_Register',
      expect.objectContaining({ userName: 'alice', password: 'pw' }),
      expect.any(Object)
    );
  });

  it('uses hashedPassword when salt is provided', () => {
    register({ userName: 'alice', password: 'pw' } as any, 'salt');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_Register',
      expect.objectContaining({ hashedPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('RespRegistrationAccepted calls login without salt and registrationSuccess', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(10);
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), undefined);
    expect(SessionPersistence.registrationSuccess).toHaveBeenCalled();
  });

  it('RespRegistrationAccepted forwards salt to login', () => {
    register({ userName: 'alice', password: 'pw' } as any, 'mySalt');
    invokeResponseCode(10);
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'mySalt');
    expect(SessionPersistence.registrationSuccess).toHaveBeenCalled();
  });

  it('RespRegistrationAcceptedNeedsActivation calls accountAwaitingActivation', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(11);
    expect(SessionPersistence.accountAwaitingActivation).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('RespUserAlreadyExists calls registrationUserNameError', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(12);
    expect(SessionPersistence.registrationUserNameError).toHaveBeenCalled();
  });

  it('RespUsernameInvalid calls registrationUserNameError', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(3);
    expect(SessionPersistence.registrationUserNameError).toHaveBeenCalled();
  });

  it('RespPasswordTooShort calls registrationPasswordError', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(13);
    expect(SessionPersistence.registrationPasswordError).toHaveBeenCalled();
  });

  it('RespEmailRequiredToRegister calls registrationRequiresEmail', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(14);
    expect(SessionPersistence.registrationRequiresEmail).toHaveBeenCalled();
  });

  it('RespEmailBlackListed calls registrationEmailError', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(15);
    expect(SessionPersistence.registrationEmailError).toHaveBeenCalled();
  });

  it('RespTooManyRequests calls registrationEmailError', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(16);
    expect(SessionPersistence.registrationEmailError).toHaveBeenCalled();
  });

  it('RespRegistrationDisabled calls registrationFailed', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(17);
    expect(SessionPersistence.registrationFailed).toHaveBeenCalled();
  });

  it('RespUserIsBanned calls registrationFailed with raw.reasonStr and raw.endTime', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeResponseCode(5, { reasonStr: 'bad user', endTime: 9999 });
    expect(SessionPersistence.registrationFailed).toHaveBeenCalledWith('bad user', 9999);
  });

  it('onError calls registrationFailed', () => {
    register({ userName: 'alice', password: 'pw' } as any);
    invokeOnError();
    expect(SessionPersistence.registrationFailed).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// activate.ts
// ----------------------------------------------------------------
describe('activate', () => {
  const { activate } = jest.requireActual('./activate');

  it('sends Command_Activate', () => {
    activate({ userName: 'alice', token: 'tok' } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_Activate', expect.any(Object), expect.any(Object));
  });

  it('RespActivationAccepted calls accountActivationSuccess and login with salt', () => {
    activate({ userName: 'alice', token: 'tok' } as any, 'salt');
    invokeResponseCode(18);
    expect(SessionPersistence.accountActivationSuccess).toHaveBeenCalled();
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'salt');
  });

  it('onError calls accountActivationFailed and disconnect', () => {
    activate({ userName: 'alice', token: 'tok' } as any);
    invokeOnError();
    expect(SessionPersistence.accountActivationFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// forgotPasswordChallenge.ts
// ----------------------------------------------------------------
describe('forgotPasswordChallenge', () => {
  const { forgotPasswordChallenge } = jest.requireActual('./forgotPasswordChallenge');

  it('sends Command_ForgotPasswordChallenge', () => {
    forgotPasswordChallenge({ userName: 'alice', email: 'a@b.com' } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_ForgotPasswordChallenge', expect.any(Object), expect.any(Object)
    );
  });

  it('onSuccess calls resetPassword and disconnect', () => {
    forgotPasswordChallenge({ userName: 'alice', email: 'a@b.com' } as any);
    invokeOnSuccess();
    expect(SessionPersistence.resetPassword).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordChallenge({ userName: 'alice', email: 'a@b.com' } as any);
    invokeOnError();
    expect(SessionPersistence.resetPasswordFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// forgotPasswordRequest.ts
// ----------------------------------------------------------------
describe('forgotPasswordRequest', () => {
  const { forgotPasswordRequest } = jest.requireActual('./forgotPasswordRequest');

  it('sends Command_ForgotPasswordRequest', () => {
    forgotPasswordRequest({ userName: 'alice' } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_ForgotPasswordRequest', expect.any(Object), expect.any(Object));
  });

  it('onSuccess with challengeEmail calls resetPasswordChallenge', () => {
    forgotPasswordRequest({ userName: 'alice' } as any);
    const resp = { challengeEmail: true };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_ForgotPasswordRequest.ext': resp });
    expect(SessionPersistence.resetPasswordChallenge).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onSuccess without challengeEmail calls resetPassword', () => {
    forgotPasswordRequest({ userName: 'alice' } as any);
    const resp = { challengeEmail: false };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_ForgotPasswordRequest.ext': resp });
    expect(SessionPersistence.resetPassword).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordRequest({ userName: 'alice' } as any);
    invokeOnError();
    expect(SessionPersistence.resetPasswordFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// forgotPasswordReset.ts
// ----------------------------------------------------------------
describe('forgotPasswordReset', () => {
  const { forgotPasswordReset } = jest.requireActual('./forgotPasswordReset');

  it('sends Command_ForgotPasswordReset with plain newPassword when no salt', () => {
    forgotPasswordReset({ userName: 'alice', token: 'tok', newPassword: 'newpw' } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_ForgotPasswordReset',
      expect.objectContaining({ newPassword: 'newpw' }),
      expect.any(Object)
    );
  });

  it('sends hashed new password when salt provided', () => {
    forgotPasswordReset({ userName: 'alice', token: 'tok', newPassword: 'newpw' } as any, 'salt');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      'Command_ForgotPasswordReset',
      expect.objectContaining({ hashedNewPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('onSuccess calls resetPasswordSuccess and disconnect', () => {
    forgotPasswordReset({ userName: 'alice', token: 'tok', newPassword: 'newpw' } as any);
    invokeOnSuccess();
    expect(SessionPersistence.resetPasswordSuccess).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordReset({ userName: 'alice', token: 'tok', newPassword: 'newpw' } as any);
    invokeOnError();
    expect(SessionPersistence.resetPasswordFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// requestPasswordSalt.ts
// ----------------------------------------------------------------
describe('requestPasswordSalt', () => {
  const { requestPasswordSalt } = jest.requireActual('./requestPasswordSalt');

  it('sends Command_RequestPasswordSalt', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.LOGIN } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith('Command_RequestPasswordSalt', expect.any(Object), expect.any(Object));
  });

  it('onSuccess with LOGIN reason calls login', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.LOGIN } as any);
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_PasswordSalt.ext': resp });
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'salt123');
  });

  it('onSuccess with ACTIVATE_ACCOUNT reason calls activate', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.ACTIVATE_ACCOUNT } as any);
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_PasswordSalt.ext': resp });
    expect(SessionIndexMocks.activate).toHaveBeenCalledWith(expect.any(Object), 'salt123');
  });

  it('onSuccess with PASSWORD_RESET reason calls forgotPasswordReset', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.PASSWORD_RESET } as any);
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_PasswordSalt.ext': resp });
    expect(SessionIndexMocks.forgotPasswordReset).toHaveBeenCalled();
  });

  it('onResponseCode RespRegistrationRequired calls updateStatus and disconnect', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.LOGIN } as any);
    invokeResponseCode(6);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, expect.any(String));
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onResponseCode RespRegistrationRequired with ACTIVATE_ACCOUNT calls accountActivationFailed', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.ACTIVATE_ACCOUNT } as any);
    invokeResponseCode(6);
    expect(SessionPersistence.accountActivationFailed).toHaveBeenCalled();
  });

  it('onError calls updateStatus DISCONNECTED and disconnect', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.LOGIN } as any);
    invokeOnError();
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError with PASSWORD_RESET reason calls resetPasswordFailed', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.PASSWORD_RESET } as any);
    invokeOnError();
    expect(SessionPersistence.resetPasswordFailed).toHaveBeenCalled();
  });
});
