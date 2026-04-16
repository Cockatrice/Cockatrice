import {
  WebClient,
  StatusEnum,
  SessionCommands,
  WebSocketConnectReason,
  setPendingOptions,
} from '@app/websocket';
import type {
  IAuthenticationRequest,
  AuthRequestMap,
  LoginConnectOptions,
  TestConnectionOptions,
  RegisterConnectOptions,
  ActivateConnectOptions,
  PasswordResetRequestConnectOptions,
  PasswordResetChallengeConnectOptions,
  PasswordResetConnectOptions,
} from '@app/websocket';

interface AppAuthRequestOverrides extends AuthRequestMap {
  LoginParams: Omit<LoginConnectOptions, 'reason'>;
  ConnectTarget: Omit<TestConnectionOptions, 'reason'>;
  RegisterParams: Omit<RegisterConnectOptions, 'reason'>;
  ActivateParams: Omit<ActivateConnectOptions, 'reason'>;
  ForgotPasswordRequestParams: Omit<PasswordResetRequestConnectOptions, 'reason'>;
  ForgotPasswordChallengeParams: Omit<PasswordResetChallengeConnectOptions, 'reason'>;
  ForgotPasswordResetParams: Omit<PasswordResetConnectOptions, 'reason'>;
}

export class AuthenticationRequestImpl implements IAuthenticationRequest<AppAuthRequestOverrides> {
  login(options: Omit<LoginConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebSocketConnectReason.LOGIN });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  testConnection(options: Omit<TestConnectionOptions, 'reason'>): void {
    WebClient.instance.testConnect({ host: options.host, port: options.port });
  }

  register(options: Omit<RegisterConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebSocketConnectReason.REGISTER });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  activateAccount(options: Omit<ActivateConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebSocketConnectReason.ACTIVATE_ACCOUNT });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPasswordRequest(options: Omit<PasswordResetRequestConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebSocketConnectReason.PASSWORD_RESET_REQUEST });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPasswordChallenge(options: Omit<PasswordResetChallengeConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebSocketConnectReason.PASSWORD_RESET_CHALLENGE });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPassword(options: Omit<PasswordResetConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebSocketConnectReason.PASSWORD_RESET });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  disconnect(): void {
    SessionCommands.disconnect();
  }
}
