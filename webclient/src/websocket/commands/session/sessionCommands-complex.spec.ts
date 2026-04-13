// Tests for complex session commands that call webClient directly
// or have multiple branching callbacks.

vi.mock('../../services/BackendService', () => ({
  BackendService: {
    sendSessionCommand: vi.fn(),
  },
}));

vi.mock('../../persistence', async () => {
  const { makeSessionPersistenceMock } = await import('../../__mocks__/sessionCommandMocks');
  return {
    SessionPersistence: makeSessionPersistenceMock(),
    RoomPersistence: {},
  };
});

vi.mock('../../WebClient', async () => {
  const { makeWebClientMock } = await import('../../__mocks__/sessionCommandMocks');
  return { __esModule: true, default: makeWebClientMock() };
});

vi.mock('../../utils', async () => {
  const { makeUtilsMock } = await import('../../__mocks__/sessionCommandMocks');
  return makeUtilsMock();
});

// Intercept all re-exported commands to avoid recursive real invocations
vi.mock('./', async () => {
  const { makeSessionBarrelMock } = await import('../../__mocks__/sessionCommandMocks');
  return makeSessionBarrelMock();
});

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';
import webClient from '../../WebClient';
import * as SessionIndexMocks from './';
import { StatusEnum, WebSocketConnectReason } from 'types';
import { hashPassword, generateSalt, passwordSaltSupported } from '../../utils';
import { Response_ResponseCode } from 'generated/proto/response_pb';
import {
  Command_Activate_ext,
  Command_ForgotPasswordChallenge_ext,
  Command_ForgotPasswordRequest_ext,
  Command_ForgotPasswordReset_ext,
  Command_Login_ext,
  Command_Register_ext,
  Command_RequestPasswordSalt_ext,
} from 'generated/proto/session_commands_pb';
import { Response_ForgotPasswordRequest_ext } from 'generated/proto/response_forgotpasswordrequest_pb';
import { Response_Login_ext } from 'generated/proto/response_login_pb';
import { Response_PasswordSalt_ext } from 'generated/proto/response_password_salt_pb';
import { connect } from './connect';
import { updateStatus } from './updateStatus';
import { login } from './login';
import { register } from './register';
import { activate } from './activate';
import { forgotPasswordChallenge } from './forgotPasswordChallenge';
import { forgotPasswordRequest } from './forgotPasswordRequest';
import { forgotPasswordReset } from './forgotPasswordReset';
import { requestPasswordSalt } from './requestPasswordSalt';

const { getLastSendOpts, invokeOnSuccess, invokeResponseCode, invokeOnError } = makeCallbackHelpers(
  BackendService.sendSessionCommand as vi.Mock,
  2
);

beforeEach(() => {
  vi.clearAllMocks();
  (hashPassword as vi.Mock).mockReturnValue('hashed_pw');
  (generateSalt as vi.Mock).mockReturnValue('randSalt');
  (passwordSaltSupported as vi.Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------
// connect.ts
// ----------------------------------------------------------------
describe('connect', () => {

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

  it('sends Command_Login with plain password when no salt', () => {
    login({ userName: 'alice' } as any, 'pw');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_Login_ext,
      expect.objectContaining({ password: 'pw' }),
      expect.objectContaining({ responseExt: Response_Login_ext })
    );
  });

  it('sends Command_Login with hashedPassword when salt is given', () => {
    login({ userName: 'alice' } as any, 'pw', 'salt');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_Login_ext,
      expect.objectContaining({ hashedPassword: 'hashed_pw' }),
      expect.objectContaining({ responseExt: Response_Login_ext })
    );
  });

  it('uses options.hashedPassword if provided', () => {
    login({ userName: 'alice', hashedPassword: 'pre_hashed' } as any, 'pw', 'salt');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_Login_ext,
      expect.objectContaining({ hashedPassword: 'pre_hashed' }),
      expect.objectContaining({ responseExt: Response_Login_ext })
    );
  });

  it('onSuccess dispatches buddy/ignore/user and calls listUsers/listRooms', () => {
    login({ userName: 'alice' } as any, 'pw');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    expect(SessionPersistence.updateBuddyList).toHaveBeenCalledWith([]);
    expect(SessionPersistence.updateIgnoreList).toHaveBeenCalledWith([]);
    expect(SessionPersistence.updateUser).toHaveBeenCalledWith({ name: 'alice' });
    expect(SessionPersistence.loginSuccessful).toHaveBeenCalled();
    expect(SessionIndexMocks.listUsers).toHaveBeenCalled();
    expect(SessionIndexMocks.listRooms).toHaveBeenCalled();
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.LOGGED_IN, 'Logged in.');
  });

  it('onSuccess does NOT pass plaintext password to loginSuccessful', () => {
    login({ userName: 'alice' } as any, 'secret');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    const calledWith = (SessionPersistence.loginSuccessful as vi.Mock).mock.calls[0][0];
    expect(calledWith).not.toHaveProperty('password');
  });

  it('onSuccess passes hashedPassword to loginSuccessful when salt is used', () => {
    login({ userName: 'alice' } as any, 'pw', 'salt');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    const calledWith = (SessionPersistence.loginSuccessful as vi.Mock).mock.calls[0][0];
    expect(calledWith).toHaveProperty('hashedPassword', 'hashed_pw');
  });

  it('onResponseCode RespClientUpdateRequired calls onLoginError', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespClientUpdateRequired);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onResponseCode RespWrongPassword', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespWrongPassword);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespUsernameInvalid', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespUsernameInvalid);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespWouldOverwriteOldSession', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespWouldOverwriteOldSession);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespUserIsBanned', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespUserIsBanned);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespRegistrationRequired', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationRequired);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespClientIdRequired', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespClientIdRequired);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespContextError', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespContextError);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespAccountNotActivated calls accountAwaitingActivation without password in options', () => {
    login({ userName: 'alice', password: 'leaked' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespAccountNotActivated);
    expect(SessionPersistence.accountAwaitingActivation).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() })
    );
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onError calls onLoginError with unknown error message', () => {
    login({ userName: 'alice' } as any, 'pw');
    invokeOnError(999);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// register.ts
// ----------------------------------------------------------------
describe('register', () => {

  it('sends Command_Register with plain password when no salt', () => {
    register({ userName: 'alice', email: 'a@b.com', country: 'US', realName: 'Al' } as any, 'pw');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_Register_ext,
      expect.objectContaining({ password: 'pw' }),
      expect.any(Object)
    );
  });

  it('uses hashedPassword when salt is provided', () => {
    register({ userName: 'alice' } as any, 'pw', 'salt');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_Register_ext,
      expect.objectContaining({ hashedPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('RespRegistrationAccepted calls login without salt and registrationSuccess', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationAccepted);
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', undefined);
    expect(SessionPersistence.registrationSuccess).toHaveBeenCalled();
  });

  it('RespRegistrationAccepted forwards salt to login', () => {
    register({ userName: 'alice' } as any, 'pw', 'mySalt');
    invokeResponseCode(Response_ResponseCode.RespRegistrationAccepted);
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', 'mySalt');
    expect(SessionPersistence.registrationSuccess).toHaveBeenCalled();
  });

  it('RespRegistrationAcceptedNeedsActivation calls accountAwaitingActivation without password in options', () => {
    register({ userName: 'alice', password: 'leaked' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationAcceptedNeedsActivation);
    expect(SessionPersistence.accountAwaitingActivation).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() })
    );
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('RespUserAlreadyExists calls registrationUserNameError', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespUserAlreadyExists);
    expect(SessionPersistence.registrationUserNameError).toHaveBeenCalled();
  });

  it('RespUsernameInvalid calls registrationUserNameError', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespUsernameInvalid);
    expect(SessionPersistence.registrationUserNameError).toHaveBeenCalled();
  });

  it('RespPasswordTooShort calls registrationPasswordError', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespPasswordTooShort);
    expect(SessionPersistence.registrationPasswordError).toHaveBeenCalled();
  });

  it('RespEmailRequiredToRegister calls registrationRequiresEmail', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespEmailRequiredToRegister);
    expect(SessionPersistence.registrationRequiresEmail).toHaveBeenCalled();
  });

  it('RespEmailBlackListed calls registrationEmailError', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespEmailBlackListed);
    expect(SessionPersistence.registrationEmailError).toHaveBeenCalled();
  });

  it('RespTooManyRequests calls registrationEmailError', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespTooManyRequests);
    expect(SessionPersistence.registrationEmailError).toHaveBeenCalled();
  });

  it('RespRegistrationDisabled calls registrationFailed', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationDisabled);
    expect(SessionPersistence.registrationFailed).toHaveBeenCalled();
  });

  it('RespUserIsBanned calls registrationFailed with raw.reasonStr and raw.endTime', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespUserIsBanned, { reasonStr: 'bad user', endTime: 9999 });
    expect(SessionPersistence.registrationFailed).toHaveBeenCalledWith('bad user', 9999);
  });

  it('onError calls registrationFailed', () => {
    register({ userName: 'alice' } as any, 'pw');
    invokeOnError();
    expect(SessionPersistence.registrationFailed).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// activate.ts
// ----------------------------------------------------------------
describe('activate', () => {

  it('sends Command_Activate with userName and token, not password', () => {
    activate({ userName: 'alice', token: 'tok' } as any, 'pw');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_Activate_ext,
      expect.objectContaining({ userName: 'alice', token: 'tok' }),
      expect.any(Object)
    );
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_Activate_ext,
      expect.not.objectContaining({ password: expect.anything() }),
      expect.any(Object)
    );
  });

  it('RespActivationAccepted calls accountActivationSuccess and forwards password+salt to login', () => {
    activate({ userName: 'alice', token: 'tok' } as any, 'pw', 'salt');
    invokeResponseCode(Response_ResponseCode.RespActivationAccepted);
    expect(SessionPersistence.accountActivationSuccess).toHaveBeenCalled();
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', 'salt');
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

  it('sends Command_ForgotPasswordChallenge', () => {
    forgotPasswordChallenge({ userName: 'alice', email: 'a@b.com' } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ForgotPasswordChallenge_ext, expect.any(Object), expect.any(Object)
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

  it('sends Command_ForgotPasswordRequest', () => {
    forgotPasswordRequest({ userName: 'alice' } as any);
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ForgotPasswordRequest_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_ForgotPasswordRequest_ext })
    );
  });

  it('onSuccess with challengeEmail calls resetPasswordChallenge', () => {
    forgotPasswordRequest({ userName: 'alice' } as any);
    const resp = { challengeEmail: true };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.resetPasswordChallenge).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onSuccess without challengeEmail calls resetPassword', () => {
    forgotPasswordRequest({ userName: 'alice' } as any);
    const resp = { challengeEmail: false };
    invokeOnSuccess(resp, { responseCode: 0 });
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

  it('sends Command_ForgotPasswordReset with plain newPassword when no salt', () => {
    forgotPasswordReset({ userName: 'alice', token: 'tok' } as any, 'newpw');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ForgotPasswordReset_ext,
      expect.objectContaining({ newPassword: 'newpw' }),
      expect.any(Object)
    );
  });

  it('sends hashed new password when salt provided', () => {
    forgotPasswordReset({ userName: 'alice', token: 'tok' } as any, 'newpw', 'salt');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_ForgotPasswordReset_ext,
      expect.objectContaining({ hashedNewPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('onSuccess calls resetPasswordSuccess and disconnect', () => {
    forgotPasswordReset({ userName: 'alice', token: 'tok' } as any, 'newpw');
    invokeOnSuccess();
    expect(SessionPersistence.resetPasswordSuccess).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordReset({ userName: 'alice', token: 'tok' } as any, 'newpw');
    invokeOnError();
    expect(SessionPersistence.resetPasswordFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// requestPasswordSalt.ts
// ----------------------------------------------------------------
describe('requestPasswordSalt', () => {

  it('sends Command_RequestPasswordSalt', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.LOGIN } as any, 'pw');
    expect(BackendService.sendSessionCommand).toHaveBeenCalledWith(
      Command_RequestPasswordSalt_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_PasswordSalt_ext })
    );
  });

  it('onSuccess with LOGIN reason forwards password+salt to login', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.LOGIN } as any, 'pw');
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', 'salt123');
  });

  it('onSuccess with ACTIVATE_ACCOUNT reason forwards password+salt to activate', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.ACTIVATE_ACCOUNT } as any, 'pw');
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionIndexMocks.activate).toHaveBeenCalledWith(expect.any(Object), 'pw', 'salt123');
  });

  it('onSuccess with PASSWORD_RESET reason forwards newPassword+salt to forgotPasswordReset', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.PASSWORD_RESET } as any, undefined, 'newpw');
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionIndexMocks.forgotPasswordReset).toHaveBeenCalledWith(expect.any(Object), 'newpw', 'salt123');
  });

  it('onResponseCode RespRegistrationRequired calls updateStatus and disconnect', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.LOGIN } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationRequired);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, expect.any(String));
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onResponseCode RespRegistrationRequired with ACTIVATE_ACCOUNT calls accountActivationFailed', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.ACTIVATE_ACCOUNT } as any, 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationRequired);
    expect(SessionPersistence.accountActivationFailed).toHaveBeenCalled();
  });

  it('onError calls updateStatus DISCONNECTED and disconnect', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.LOGIN } as any, 'pw');
    invokeOnError();
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError with PASSWORD_RESET reason calls resetPasswordFailed', () => {
    requestPasswordSalt({ userName: 'alice', reason: WebSocketConnectReason.PASSWORD_RESET } as any, undefined, 'newpw');
    invokeOnError();
    expect(SessionPersistence.resetPasswordFailed).toHaveBeenCalled();
  });
});
