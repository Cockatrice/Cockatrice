// Tests for complex session commands that call webClient directly
// or have multiple branching callbacks.

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

import { Mock } from 'vitest';
import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { SessionPersistence } from '../../persistence';
import webClient from '../../WebClient';
import * as SessionIndexMocks from './';
import { App, Enriched, Data } from '@app/types';
import { hashPassword, generateSalt, passwordSaltSupported } from '../../utils';

import { create, setExtension } from '@bufbuild/protobuf';

import { connect } from './connect';
import { updateStatus } from './updateStatus';
import { login } from './login';
import { register } from './register';
import { activate } from './activate';
import { forgotPasswordChallenge } from './forgotPasswordChallenge';
import { forgotPasswordRequest } from './forgotPasswordRequest';
import { forgotPasswordReset } from './forgotPasswordReset';
import { requestPasswordSalt } from './requestPasswordSalt';

const { invokeOnSuccess, invokeResponseCode, invokeOnError } = makeCallbackHelpers(
  webClient.protobuf.sendSessionCommand as Mock,
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
const makeSaltOpts = (
  reason: App.WebSocketConnectReason,
  extras: Record<string, unknown> = {}
) => ({ ...baseTransport, userName: 'alice', reason, ...extras } as
  | Enriched.LoginConnectOptions
  | Enriched.ActivateConnectOptions
  | Enriched.PasswordResetConnectOptions);

beforeEach(() => {
  (hashPassword as Mock).mockReturnValue('hashed_pw');
  (generateSalt as Mock).mockReturnValue('randSalt');
  (passwordSaltSupported as Mock).mockReturnValue(0);
});

// ----------------------------------------------------------------
// connect.ts
// ----------------------------------------------------------------
describe('connect', () => {

  it('calls updateStatus CONNECTING for LOGIN reason', () => {
    connect({ host: 'h', port: '1', userName: 'u', reason: App.WebSocketConnectReason.LOGIN });
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTING, 'Connecting...');
    expect(webClient.connect).toHaveBeenCalled();
  });

  it('calls updateStatus CONNECTING for REGISTER reason', () => {
    connect(makeRegisterOpts({ userName: 'u', realName: 'U' }));
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls updateStatus CONNECTING for ACTIVATE_ACCOUNT reason', () => {
    connect({ host: 'h', port: '1', userName: 'u', token: 'tok', reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT });
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls updateStatus CONNECTING for PASSWORD_RESET_REQUEST reason', () => {
    connect({ host: 'h', port: '1', userName: 'u', reason: App.WebSocketConnectReason.PASSWORD_RESET_REQUEST });
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls updateStatus CONNECTING for PASSWORD_RESET_CHALLENGE reason', () => {
    connect({ host: 'h', port: '1', userName: 'u', email: 'a@b.com', reason: App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE });
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls updateStatus CONNECTING for PASSWORD_RESET reason', () => {
    connect({ host: 'h', port: '1', userName: 'u', token: 'tok', newPassword: 'newpw', reason: App.WebSocketConnectReason.PASSWORD_RESET });
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTING, 'Connecting...');
  });

  it('calls testConnect for TEST_CONNECTION reason', () => {
    connect({ host: 'h', port: '1', reason: App.WebSocketConnectReason.TEST_CONNECTION });
    expect(webClient.testConnect).toHaveBeenCalled();
    expect(webClient.connect).not.toHaveBeenCalled();
  });

  it('calls updateStatus DISCONNECTED for unknown reason', () => {
    const bogus = { host: 'h', port: '1', reason: 999 as App.WebSocketConnectReason };
    connect(bogus as Enriched.WebSocketConnectOptions);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.DISCONNECTED, expect.stringContaining('Unknown'));
  });
});

// ----------------------------------------------------------------
// updateStatus.ts
// ----------------------------------------------------------------
describe('updateStatus', () => {

  it('calls SessionPersistence.updateStatus and webClient.updateStatus', () => {
    updateStatus(App.StatusEnum.CONNECTED, 'OK');
    expect(SessionPersistence.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTED, 'OK');
    expect(webClient.updateStatus).toHaveBeenCalledWith(App.StatusEnum.CONNECTED);
  });
});

// ----------------------------------------------------------------
// login.ts
// ----------------------------------------------------------------
describe('login', () => {

  it('sends Command_Login with plain password when no salt', () => {
    login(makeLoginOpts(), 'pw');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_Login_ext,
      expect.objectContaining({ password: 'pw' }),
      expect.objectContaining({ responseExt: Data.Response_Login_ext })
    );
  });

  it('sends Command_Login with hashedPassword when salt is given', () => {
    login(makeLoginOpts(), 'pw', 'salt');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_Login_ext,
      expect.objectContaining({ hashedPassword: 'hashed_pw' }),
      expect.objectContaining({ responseExt: Data.Response_Login_ext })
    );
  });

  it('uses options.hashedPassword if provided', () => {
    login(makeLoginOpts({ hashedPassword: 'pre_hashed' }), 'pw', 'salt');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_Login_ext,
      expect.objectContaining({ hashedPassword: 'pre_hashed' }),
      expect.objectContaining({ responseExt: Data.Response_Login_ext })
    );
  });

  it('onSuccess dispatches buddy/ignore/user and calls listUsers/listRooms', () => {
    login(makeLoginOpts(), 'pw');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    expect(SessionPersistence.updateBuddyList).toHaveBeenCalledWith([]);
    expect(SessionPersistence.updateIgnoreList).toHaveBeenCalledWith([]);
    expect(SessionPersistence.updateUser).toHaveBeenCalledWith({ name: 'alice' });
    expect(SessionPersistence.loginSuccessful).toHaveBeenCalled();
    expect(SessionIndexMocks.listUsers).toHaveBeenCalled();
    expect(SessionIndexMocks.listRooms).toHaveBeenCalled();
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.LOGGED_IN, 'Logged in.');
  });

  it('onSuccess does NOT pass plaintext password to loginSuccessful', () => {
    login(makeLoginOpts(), 'secret');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    const calledWith = (SessionPersistence.loginSuccessful as Mock).mock.calls[0][0];
    expect(calledWith).not.toHaveProperty('password');
  });

  it('onSuccess passes hashedPassword to loginSuccessful when salt is used', () => {
    login({ host: 'h', port: '1', userName: 'alice', reason: App.WebSocketConnectReason.LOGIN }, 'pw', 'salt');
    const loginResp = { buddyList: [], ignoreList: [], userInfo: { name: 'alice' } };
    invokeOnSuccess(loginResp, { responseCode: 0 });
    const calledWith = (SessionPersistence.loginSuccessful as Mock).mock.calls[0][0];
    expect(calledWith).toHaveProperty('hashedPassword', 'hashed_pw');
  });

  it('onResponseCode RespClientUpdateRequired calls onLoginError', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespClientUpdateRequired);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onResponseCode RespWrongPassword', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespWrongPassword);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespUsernameInvalid', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespUsernameInvalid);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespWouldOverwriteOldSession', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespWouldOverwriteOldSession);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespUserIsBanned', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespUserIsBanned);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespRegistrationRequired', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespRegistrationRequired);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespClientIdRequired', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespClientIdRequired);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespContextError', () => {
    login(makeLoginOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespContextError);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onResponseCode RespAccountNotActivated calls accountAwaitingActivation without password in options', () => {
    login(makeLoginOpts({ password: 'leaked' }), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespAccountNotActivated);
    expect(SessionPersistence.accountAwaitingActivation).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() })
    );
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });

  it('onError calls onLoginError with unknown error message', () => {
    login(makeLoginOpts(), 'pw');
    invokeOnError(999);
    expect(SessionPersistence.loginFailed).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// register.ts
// ----------------------------------------------------------------
describe('register', () => {

  it('sends Command_Register with plain password when no salt', () => {
    register(makeRegisterOpts(), 'pw');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_Register_ext,
      expect.objectContaining({ password: 'pw' }),
      expect.any(Object)
    );
  });

  it('uses hashedPassword when salt is provided', () => {
    register(makeRegisterOpts(), 'pw', 'salt');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_Register_ext,
      expect.objectContaining({ hashedPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('RespRegistrationAccepted calls login without salt and registrationSuccess', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespRegistrationAccepted);
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', undefined);
    expect(SessionPersistence.registrationSuccess).toHaveBeenCalled();
  });

  it('RespRegistrationAccepted forwards salt to login', () => {
    register(makeRegisterOpts(), 'pw', 'mySalt');
    invokeResponseCode(Data.Response_ResponseCode.RespRegistrationAccepted);
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', 'mySalt');
    expect(SessionPersistence.registrationSuccess).toHaveBeenCalled();
  });

  it('RespRegistrationAcceptedNeedsActivation calls accountAwaitingActivation without password in options', () => {
    register(makeRegisterOpts({ password: 'leaked' }), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespRegistrationAcceptedNeedsActivation);
    expect(SessionPersistence.accountAwaitingActivation).toHaveBeenCalledWith(
      expect.not.objectContaining({ password: expect.anything() })
    );
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('RespUserAlreadyExists calls registrationUserNameError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespUserAlreadyExists);
    expect(SessionPersistence.registrationUserNameError).toHaveBeenCalled();
  });

  it('RespUsernameInvalid calls registrationUserNameError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespUsernameInvalid);
    expect(SessionPersistence.registrationUserNameError).toHaveBeenCalled();
  });

  it('RespPasswordTooShort calls registrationPasswordError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespPasswordTooShort);
    expect(SessionPersistence.registrationPasswordError).toHaveBeenCalled();
  });

  it('RespEmailRequiredToRegister calls registrationRequiresEmail', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespEmailRequiredToRegister);
    expect(SessionPersistence.registrationRequiresEmail).toHaveBeenCalled();
  });

  it('RespEmailBlackListed calls registrationEmailError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespEmailBlackListed);
    expect(SessionPersistence.registrationEmailError).toHaveBeenCalled();
  });

  it('RespTooManyRequests calls registrationEmailError', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespTooManyRequests);
    expect(SessionPersistence.registrationEmailError).toHaveBeenCalled();
  });

  it('RespRegistrationDisabled calls registrationFailed', () => {
    register(makeRegisterOpts(), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespRegistrationDisabled);
    expect(SessionPersistence.registrationFailed).toHaveBeenCalled();
  });

  it('RespUserIsBanned calls registrationFailed with deniedReasonStr and deniedEndTime', () => {
    register(makeRegisterOpts(), 'pw');
    const raw = create(Data.ResponseSchema, { responseCode: Data.Response_ResponseCode.RespUserIsBanned });
    setExtension(raw, Data.Response_Register_ext, create(Data.Response_RegisterSchema, {
      deniedReasonStr: 'bad user', deniedEndTime: 9999n,
    }));
    invokeResponseCode(Data.Response_ResponseCode.RespUserIsBanned, raw);
    expect(SessionPersistence.registrationFailed).toHaveBeenCalledWith('bad user', 9999);
  });

  it('onError calls registrationFailed', () => {
    register(makeRegisterOpts(), 'pw');
    invokeOnError();
    expect(SessionPersistence.registrationFailed).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// activate.ts
// ----------------------------------------------------------------
describe('activate', () => {

  it('sends Command_Activate with userName and token, not password', () => {
    activate(makeActivateOpts(), 'pw');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_Activate_ext,
      expect.objectContaining({ userName: 'alice', token: 'tok' }),
      expect.any(Object)
    );
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_Activate_ext,
      expect.not.objectContaining({ password: expect.anything() }),
      expect.any(Object)
    );
  });

  it('RespActivationAccepted calls accountActivationSuccess and forwards password+salt to login', () => {
    activate(makeActivateOpts(), 'pw', 'salt');
    invokeResponseCode(Data.Response_ResponseCode.RespActivationAccepted);
    expect(SessionPersistence.accountActivationSuccess).toHaveBeenCalled();
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', 'salt');
  });

  it('onError calls accountActivationFailed and disconnect', () => {
    activate(makeActivateOpts());
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
    forgotPasswordChallenge(makeForgotChallengeOpts());
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ForgotPasswordChallenge_ext, expect.any(Object), expect.any(Object)
    );
  });

  it('onSuccess calls resetPassword and disconnect', () => {
    forgotPasswordChallenge(makeForgotChallengeOpts());
    invokeOnSuccess();
    expect(SessionPersistence.resetPassword).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordChallenge(makeForgotChallengeOpts());
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
    forgotPasswordRequest(makeForgotRequestOpts());
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ForgotPasswordRequest_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_ForgotPasswordRequest_ext })
    );
  });

  it('onSuccess with challengeEmail calls resetPasswordChallenge', () => {
    forgotPasswordRequest(makeForgotRequestOpts());
    const resp = { challengeEmail: true };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.resetPasswordChallenge).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onSuccess without challengeEmail calls resetPassword', () => {
    forgotPasswordRequest(makeForgotRequestOpts());
    const resp = { challengeEmail: false };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionPersistence.resetPassword).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordRequest(makeForgotRequestOpts());
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
    forgotPasswordReset(makeForgotResetOpts(), 'newpw');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ForgotPasswordReset_ext,
      expect.objectContaining({ newPassword: 'newpw' }),
      expect.any(Object)
    );
  });

  it('sends hashed new password when salt provided', () => {
    forgotPasswordReset(makeForgotResetOpts(), 'newpw', 'salt');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_ForgotPasswordReset_ext,
      expect.objectContaining({ hashedNewPassword: 'hashed_pw' }),
      expect.any(Object)
    );
  });

  it('onSuccess calls resetPasswordSuccess and disconnect', () => {
    forgotPasswordReset(makeForgotResetOpts(), 'newpw');
    invokeOnSuccess();
    expect(SessionPersistence.resetPasswordSuccess).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError calls resetPasswordFailed and disconnect', () => {
    forgotPasswordReset(makeForgotResetOpts(), 'newpw');
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
    requestPasswordSalt(makeSaltOpts(App.WebSocketConnectReason.LOGIN), 'pw');
    expect(webClient.protobuf.sendSessionCommand).toHaveBeenCalledWith(
      Data.Command_RequestPasswordSalt_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Data.Response_PasswordSalt_ext })
    );
  });

  it('onSuccess with LOGIN reason forwards password+salt to login', () => {
    requestPasswordSalt(makeSaltOpts(App.WebSocketConnectReason.LOGIN), 'pw');
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionIndexMocks.login).toHaveBeenCalledWith(expect.any(Object), 'pw', 'salt123');
  });

  it('onSuccess with ACTIVATE_ACCOUNT reason forwards password+salt to activate', () => {
    requestPasswordSalt(makeSaltOpts(App.WebSocketConnectReason.ACTIVATE_ACCOUNT, { token: 'tok' }), 'pw');
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionIndexMocks.activate).toHaveBeenCalledWith(expect.any(Object), 'pw', 'salt123');
  });

  it('onSuccess with PASSWORD_RESET reason forwards newPassword+salt to forgotPasswordReset', () => {
    requestPasswordSalt(
      makeSaltOpts(App.WebSocketConnectReason.PASSWORD_RESET, { token: 'tok', newPassword: 'newpw' }),
      undefined,
      'newpw'
    );
    const resp = { passwordSalt: 'salt123' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(SessionIndexMocks.forgotPasswordReset).toHaveBeenCalledWith(expect.any(Object), 'newpw', 'salt123');
  });

  it('onResponseCode RespRegistrationRequired calls updateStatus and disconnect', () => {
    requestPasswordSalt(makeSaltOpts(App.WebSocketConnectReason.LOGIN), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespRegistrationRequired);
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalledWith(App.StatusEnum.DISCONNECTED, expect.any(String));
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onResponseCode RespRegistrationRequired with ACTIVATE_ACCOUNT calls accountActivationFailed', () => {
    requestPasswordSalt(makeSaltOpts(App.WebSocketConnectReason.ACTIVATE_ACCOUNT, { token: 'tok' }), 'pw');
    invokeResponseCode(Data.Response_ResponseCode.RespRegistrationRequired);
    expect(SessionPersistence.accountActivationFailed).toHaveBeenCalled();
  });

  it('onError calls updateStatus DISCONNECTED and disconnect', () => {
    requestPasswordSalt(makeSaltOpts(App.WebSocketConnectReason.LOGIN), 'pw');
    invokeOnError();
    expect(SessionIndexMocks.updateStatus).toHaveBeenCalled();
    expect(SessionIndexMocks.disconnect).toHaveBeenCalled();
  });

  it('onError with PASSWORD_RESET reason calls resetPasswordFailed', () => {
    requestPasswordSalt(
      makeSaltOpts(App.WebSocketConnectReason.PASSWORD_RESET, { token: 'tok', newPassword: 'newpw' }),
      undefined,
      'newpw'
    );
    invokeOnError();
    expect(SessionPersistence.resetPasswordFailed).toHaveBeenCalled();
  });
});
