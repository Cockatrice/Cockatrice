// Tests for complex session commands that call WebClient directly
// or have multiple branching callbacks.

vi.mock('../../WebClient', async () => {
  const { makeWebClientMock } = await import('../../__mocks__/sessionCommandMocks');
  return { WebClient: { instance: makeWebClientMock() } };
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

import { Mock } from 'vitest';
import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { useWebClientCleanup } from '../../__mocks__/helpers';
import { WebClient } from '../../WebClient';
import * as SessionIndexMocks from './';
import { App, Enriched } from '@app/types';
import { StatusEnum } from '../../StatusEnum';
import {
  Command_Activate_ext,
  Command_ForgotPasswordChallenge_ext,
  Command_ForgotPasswordRequest_ext,
  Command_ForgotPasswordReset_ext,
  Command_Login_ext,
  Command_Register_ext,
  Command_RequestPasswordSalt_ext,
  Response_ForgotPasswordRequest_ext,
  Response_Login_ext,
  Response_PasswordSalt_ext,
  Response_Register_ext,
  Response_RegisterSchema,
  Response_ResponseCode,
  ResponseSchema,
} from '@app/generated';
import { hashPassword, generateSalt, passwordSaltSupported } from '../../utils';

import { create, setExtension } from '@bufbuild/protobuf';

import { connect, testConnect } from './connect';
import { updateStatus } from './updateStatus';
import { login } from './login';
import { register } from './register';
import { activate } from './activate';
import { forgotPasswordChallenge } from './forgotPasswordChallenge';
import { forgotPasswordRequest } from './forgotPasswordRequest';
import { forgotPasswordReset } from './forgotPasswordReset';
import { requestPasswordSalt } from './requestPasswordSalt';

useWebClientCleanup();

const { invokeOnSuccess, invokeResponseCode, invokeOnError } = makeCallbackHelpers(
  WebClient.instance.protobuf.sendSessionCommand as Mock,
  2
);

const baseTransport = { host: 'h', port: '1' };
const makeLoginOpts = (overrides: Partial<Enriched.LoginConnectOptions> = {}): Enriched.LoginConnectOptions => ({
  ...baseTransport,
  userName: 'alice',
  reason: App.WebSocketConnectReason.LOGIN,
  ...overrides,
});
const makeRegisterOpts = (
  overrides: Partial<Enriched.RegisterConnectOptions> = {}
): Enriched.RegisterConnectOptions => ({
  ...baseTransport,
  userName: 'alice',
  password: 'pw',
  email: 'a@b.com',
  country: 'US',
  realName: 'Al',
  reason: App.WebSocketConnectReason.REGISTER,
  ...overrides,
});
const makeActivateOpts = (
  overrides: Partial<Enriched.ActivateConnectOptions> = {}
): Enriched.ActivateConnectOptions => ({
  ...baseTransport,
  userName: 'alice',
  token: 'tok',
  reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT,
  ...overrides,
});
const makeForgotRequestOpts = (): Enriched.PasswordResetRequestConnectOptions => ({
  ...baseTransport,
  userName: 'alice',
  reason: App.WebSocketConnectReason.PASSWORD_RESET_REQUEST,
});
const makeForgotChallengeOpts = (): Enriched.PasswordResetChallengeConnectOptions => ({
  ...baseTransport,
  userName: 'alice',
  email: 'a@b.com',
  reason: App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE,
});
const makeForgotResetOpts = (): Enriched.PasswordResetConnectOptions => ({
  ...baseTransport,
  userName: 'alice',
  token: 'tok',
  newPassword: 'newpw',
  reason: App.WebSocketConnectReason.PASSWORD_RESET,
});


beforeEach(() => {
  (hashPassword as Mock).mockReturnValue('hashed_pw');
  (generateSalt as Mock).mockReturnValue('randSalt');
  (passwordSaltSupported as Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------
// connect.ts
// ----------------------------------------------------------------
describe('connect', () => {

  it('calls WebClient.instance.connect with the target', () => {
    connect({ host: 'h', port: '1' });
    expect(WebClient.instance.connect).toHaveBeenCalledWith({ host: 'h', port: '1' });
  });
});

describe('testConnect', () => {

  it('calls WebClient.instance.testConnect with the target', () => {
    testConnect({ host: 'h', port: '1' });
    expect(WebClient.instance.testConnect).toHaveBeenCalledWith({ host: 'h', port: '1' });
  });
});

// ----------------------------------------------------------------
// updateStatus.ts
// ----------------------------------------------------------------
describe('updateStatus', () => {

  it('calls WebClient.instance.response.session.updateStatus and WebClient.instance.updateStatus', () => {
    updateStatus(StatusEnum.CONNECTED, 'OK');
    expect(WebClient.instance.response.session.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTED, 'OK');
    expect(WebClient.instance.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTED);
  });
});

// ----------------------------------------------------------------
// login.ts
// ----------------------------------------------------------------
describe('login', () => {

  it('sends Command_Login with plain password when no salt', () => {
    login(makeLoginOpts(), 'pw');
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_Login_ext,
      expect.objectContaining({ password: 'pw' }),
      expect.objectContaining({ responseExt: Response_Login_ext })
    );
  });

  it('sends Command_Login with hashedPassword when salt is given', () => {
    login(makeLoginOpts(), 'pw', 'salt');
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_Login_ext,
      expect.objectContaining({ hashedPassword: 'hashed_pw' }),
      expect.objectContaining({ responseExt: Response_Login_ext })
    );
  });

  it('uses options.hashedPassword if provided', () => {
    login(makeLoginOpts({ hashedPassword: 'pre_hashed' }), 'pw', 'salt');
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_Login_ext,
      expect.objectContaining({ hashedPassword: 'pre_hashed' }),
      expect.objectContaining({ responseExt: Response_Login_ext })
    );
  });

  it('onSuccess dispatches buddy/ignore/user and calls listUsers/listRooms', () => {
    login(makeLoginOpts(), 'pw');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    expect(WebClient.instance.response.session.updateBuddyList).toHaveBeenCalledWith([]);
    expect(WebClient.instance.response.session.updateIgnoreList).toHaveBeenCalledWith([]);
    expect(WebClient.instance.response.session.updateUser).toHaveBeenCalledWith({ name: 'alice' });
    expect(WebClient.instance.response.session.loginSuccessful).toHaveBeenCalled();
    expect(SessionIndexMocks.listUsers).toHaveBeenCalled();
    expect(SessionIndexMocks.listRooms).toHaveBeenCalled();
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.LOGGED_IN, 'Logged in.');
  });

  it('onSuccess does NOT pass plaintext password to loginSuccessful', () => {
    login(makeLoginOpts(), 'secret');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    const calledWith = (WebClient.instance.response.session.loginSuccessful as Mock).mock.calls[0][0];
    expect(calledWith).not.toHaveProperty('password');
  });

  it('onSuccess passes hashedPassword to loginSuccessful when salt is used', () => {
    login({ host: 'h', port: '1', userName: 'alice', reason: App.WebSocketConnectReason.LOGIN }, 'pw', 'salt');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    const calledWith = (WebClient.instance.response.session.loginSuccessful as Mock).mock.calls[0][0];
    expect(calledWith).toHaveProperty('hashedPassword', 'hashed_pw');
  });

  it('onResponseCode RespClientUpdateRequired calls onLoginError', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespClientUpdateRequired);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onResponseCode RespWrongPassword', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespWrongPassword);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespUsernameInvalid', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespUsernameInvalid);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespWouldOverwriteOldSession', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespWouldOverwriteOldSession);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespUserIsBanned', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespUserIsBanned);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespRegistrationRequired', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationRequired);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespClientIdRequired', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespClientIdRequired);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespContextError', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespContextError);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespAccountNotActivated calls accountAwaitingActivation without password in options', () => {
    login(makeLoginOpts({ password: 'leaked' }), 'pw');
    invokeResponseCode(Response_ResponseCode.RespAccountNotActivated);
    expect(WebClient.instance.response.session.accountAwaitingActivation).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() })
    );
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });

  it('onError calls onLoginError with unknown error message', () => {
    login(makeLoginOpts(), 'pw');
    invokeOnError(999);
    expect(WebClient.instance.response.session.loginFailed).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// register.ts
// ----------------------------------------------------------------
describe('register', () => {

  it('sends Command_Register with plain password when no salt', () => {
    register(makeRegisterOpts(), 'pw');
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_Register_ext,
      expect.objectContaining({ password: 'pw' }),
      expect.any(Object)
    );
  });

  it('uses hashedPassword when salt is provided', () => {
    register(makeRegisterOpts(), 'pw', 'salt');
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_Register_ext,
      expect.objectContaining({ hashedPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('RespRegistrationAccepted calls login without salt and registrationSuccess', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationAccepted);
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', undefined);
    expect(WebClient.instance.response.session.registrationSuccess).toHaveBeenCalled();
  });

  it('RespRegistrationAccepted forwards salt to login', () => {
    register(makeRegisterOpts(), 'pw', 'mySalt');
    invokeResponseCode(Response_ResponseCode.RespRegistrationAccepted);
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', 'mySalt');
    expect(WebClient.instance.response.session.registrationSuccess).toHaveBeenCalled();
  });

  it('RespRegistrationAcceptedNeedsActivation calls accountAwaitingActivation without password in options', () => {
    register(makeRegisterOpts({ password: 'leaked' }), 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationAcceptedNeedsActivation);
    expect(WebClient.instance.response.session.accountAwaitingActivation).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() })
    );
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('RespUserAlreadyExists calls registrationUserNameError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespUserAlreadyExists);
    expect(WebClient.instance.response.session.registrationUserNameError).toHaveBeenCalled();
  });

  it('RespUsernameInvalid calls registrationUserNameError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespUsernameInvalid);
    expect(WebClient.instance.response.session.registrationUserNameError).toHaveBeenCalled();
  });

  it('RespPasswordTooShort calls registrationPasswordError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespPasswordTooShort);
    expect(WebClient.instance.response.session.registrationPasswordError).toHaveBeenCalled();
  });

  it('RespEmailRequiredToRegister calls registrationRequiresEmail', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespEmailRequiredToRegister);
    expect(WebClient.instance.response.session.registrationRequiresEmail).toHaveBeenCalled();
  });

  it('RespEmailBlackListed calls registrationEmailError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespEmailBlackListed);
    expect(WebClient.instance.response.session.registrationEmailError).toHaveBeenCalled();
  });

  it('RespTooManyRequests calls registrationEmailError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespTooManyRequests);
    expect(WebClient.instance.response.session.registrationEmailError).toHaveBeenCalled();
  });

  it('RespRegistrationDisabled calls registrationFailed', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Response_ResponseCode.RespRegistrationDisabled);
    expect(WebClient.instance.response.session.registrationFailed).toHaveBeenCalled();
  });

  it('RespUserIsBanned calls registrationFailed with deniedReasonStr and deniedEndTime', () => {
    register(makeRegisterOpts(), 'pw');
    const raw = create(ResponseSchema, { responseCode: Response_ResponseCode.RespUserIsBanned });
    setExtension(raw, Response_Register_ext, create(Response_RegisterSchema, {
      deniedReasonStr: 'bad user', deniedEndTime: 9999n,
    }));
    invokeResponseCode(Response_ResponseCode.RespUserIsBanned, raw);
    expect(WebClient.instance.response.session.registrationFailed).toHaveBeenCalledWith('bad user', 9999);
  });

  it('onError calls registrationFailed', () => {
    register(makeRegisterOpts(), 'pw');
    invokeOnError();
    expect(WebClient.instance.response.session.registrationFailed).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// activate.ts
// ----------------------------------------------------------------
describe('activate', () => {

  it('sends Command_Activate with userName and token, not password', () => {
    activate(makeActivateOpts(), 'pw');
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_Activate_ext,
      expect.objectContaining({ userName: 'alice', token: 'tok' }),
      expect.any(Object)
    );
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_Activate_ext,
      expect.not.objectContaining({ password: expect.anything() }),
      expect.any(Object)
    );
  });

  it('RespActivationAccepted calls accountActivationSuccess and forwards password+salt to login', () => {
    activate(makeActivateOpts(), 'pw', 'salt');
    invokeResponseCode(Response_ResponseCode.RespActivationAccepted);
    expect(WebClient.instance.response.session.accountActivationSuccess).toHaveBeenCalled();
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', 'salt');
  });

  it('onError calls accountActivationFailed and disconnect', () => {
    activate(makeActivateOpts());
    invokeOnError();
    expect(WebClient.instance.response.session.accountActivationFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// forgotPasswordChallenge.ts
// ----------------------------------------------------------------
describe('forgotPasswordChallenge', () => {

  it('sends Command_ForgotPasswordChallenge', () => {
    forgotPasswordChallenge(makeForgotChallengeOpts());
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_ForgotPasswordChallenge_ext, expect.any(Object), expect.any(Object)
    );
  });

  it('onSuccess calls resetPassword and disconnect', () => {
    forgotPasswordChallenge(makeForgotChallengeOpts());
    invokeOnSuccess();
    expect(WebClient.instance.response.session.resetPassword).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordChallenge(makeForgotChallengeOpts());
    invokeOnError();
    expect(WebClient.instance.response.session.resetPasswordFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// forgotPasswordRequest.ts
// ----------------------------------------------------------------
describe('forgotPasswordRequest', () => {

  it('sends Command_ForgotPasswordRequest', () => {
    forgotPasswordRequest(makeForgotRequestOpts());
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_ForgotPasswordRequest_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_ForgotPasswordRequest_ext })
    );
  });

  it('onSuccess with challengeEmail calls resetPasswordChallenge', () => {
    forgotPasswordRequest(makeForgotRequestOpts());
    const resp = { challengeEmail: true };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(WebClient.instance.response.session.resetPasswordChallenge).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onSuccess without challengeEmail calls resetPassword', () => {
    forgotPasswordRequest(makeForgotRequestOpts());
    const resp = { challengeEmail: false };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(WebClient.instance.response.session.resetPassword).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordRequest(makeForgotRequestOpts());
    invokeOnError();
    expect(WebClient.instance.response.session.resetPasswordFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// forgotPasswordReset.ts
// ----------------------------------------------------------------
describe('forgotPasswordReset', () => {

  it('sends Command_ForgotPasswordReset with plain newPassword when no salt', () => {
    forgotPasswordReset(makeForgotResetOpts(), 'newpw');
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_ForgotPasswordReset_ext,
      expect.objectContaining({ newPassword: 'newpw' }),
      expect.any(Object)
    );
  });

  it('sends hashed new password when salt provided', () => {
    forgotPasswordReset(makeForgotResetOpts(), 'newpw', 'salt');
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_ForgotPasswordReset_ext,
      expect.objectContaining({ hashedNewPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('onSuccess calls resetPasswordSuccess and disconnect', () => {
    forgotPasswordReset(makeForgotResetOpts(), 'newpw');
    invokeOnSuccess();
    expect(WebClient.instance.response.session.resetPasswordSuccess).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordReset(makeForgotResetOpts(), 'newpw');
    invokeOnError();
    expect(WebClient.instance.response.session.resetPasswordFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// requestPasswordSalt.ts
// ----------------------------------------------------------------
describe('requestPasswordSalt', () => {

  it('sends Command_RequestPasswordSalt', () => {
    const onSaltReceived = vi.fn();
    const onFailure = vi.fn();
    requestPasswordSalt({ host: 'h', port: '1', userName: 'alice' }, onSaltReceived, onFailure);
    expect(WebClient.instance.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Command_RequestPasswordSalt_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_PasswordSalt_ext })
    );
  });

  it('onSuccess calls onSaltReceived with the salt', () => {
    const onSaltReceived = vi.fn();
    const onFailure = vi.fn();
    requestPasswordSalt({ host: 'h', port: '1', userName: 'alice' }, onSaltReceived, onFailure);
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(onSaltReceived).toHaveBeenCalledWith('salt123');
  });

  it('onResponseCode RespRegistrationRequired calls updateStatus and onFailure', () => {
    const onSaltReceived = vi.fn();
    const onFailure = vi.fn();
    requestPasswordSalt({ host: 'h', port: '1', userName: 'alice' }, onSaltReceived, onFailure);
    invokeResponseCode(Response_ResponseCode.RespRegistrationRequired);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTED, expect.any(String));
    expect(onFailure).toHaveBeenCalled();
  });

  it('onError calls updateStatus DISCONNECTED and onFailure', () => {
    const onSaltReceived = vi.fn();
    const onFailure = vi.fn();
    requestPasswordSalt({ host: 'h', port: '1', userName: 'alice' }, onSaltReceived, onFailure);
    invokeOnError();
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalled();
    expect(onFailure).toHaveBeenCalled();
  });
});
