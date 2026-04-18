import {
  WebClient,
  SessionCommands,
  setPendingOptions,
} from '@app/websocket';
import { WebsocketTypes } from '@app/websocket/types';

interface AppAuthRequestOverrides extends WebsocketTypes.AuthRequestMap {
  LoginParams: Omit<WebsocketTypes.LoginConnectOptions, 'reason'>;
  ConnectTarget: Omit<WebsocketTypes.TestConnectionOptions, 'reason'>;
  RegisterParams: Omit<WebsocketTypes.RegisterConnectOptions, 'reason'>;
  ActivateParams: Omit<WebsocketTypes.ActivateConnectOptions, 'reason'>;
  ForgotPasswordRequestParams: Omit<WebsocketTypes.PasswordResetRequestConnectOptions, 'reason'>;
  ForgotPasswordChallengeParams: Omit<WebsocketTypes.PasswordResetChallengeConnectOptions, 'reason'>;
  ForgotPasswordResetParams: Omit<WebsocketTypes.PasswordResetConnectOptions, 'reason'>;
}

export class AuthenticationRequestImpl implements WebsocketTypes.IAuthenticationRequest<AppAuthRequestOverrides> {
  login(options: Omit<WebsocketTypes.LoginConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebsocketTypes.WebSocketConnectReason.LOGIN });
    SessionCommands.updateStatus(WebsocketTypes.StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  testConnection(options: Omit<WebsocketTypes.TestConnectionOptions, 'reason'>): void {
    WebClient.instance.testConnect({ host: options.host, port: options.port });
  }

  register(options: Omit<WebsocketTypes.RegisterConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebsocketTypes.WebSocketConnectReason.REGISTER });
    SessionCommands.updateStatus(WebsocketTypes.StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  activateAccount(options: Omit<WebsocketTypes.ActivateConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebsocketTypes.WebSocketConnectReason.ACTIVATE_ACCOUNT });
    SessionCommands.updateStatus(WebsocketTypes.StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPasswordRequest(options: Omit<WebsocketTypes.PasswordResetRequestConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebsocketTypes.WebSocketConnectReason.PASSWORD_RESET_REQUEST });
    SessionCommands.updateStatus(WebsocketTypes.StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPasswordChallenge(options: Omit<WebsocketTypes.PasswordResetChallengeConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebsocketTypes.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE });
    SessionCommands.updateStatus(WebsocketTypes.StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPassword(options: Omit<WebsocketTypes.PasswordResetConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: WebsocketTypes.WebSocketConnectReason.PASSWORD_RESET });
    SessionCommands.updateStatus(WebsocketTypes.StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  disconnect(): void {
    SessionCommands.disconnect();
  }
}
