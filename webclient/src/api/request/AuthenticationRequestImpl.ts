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

const CONNECTING_STATUS_LABEL = 'Connecting...';

function beginConnect(
  options: { host: string; port: string | number },
  reason: WebsocketTypes.WebSocketConnectReason,
): void {
  setPendingOptions({ ...options, reason });
  SessionCommands.updateStatus(WebsocketTypes.StatusEnum.CONNECTING, CONNECTING_STATUS_LABEL);
  WebClient.instance.connect({ host: options.host, port: options.port });
}

export class AuthenticationRequestImpl implements WebsocketTypes.IAuthenticationRequest<AppAuthRequestOverrides> {
  login(options: Omit<WebsocketTypes.LoginConnectOptions, 'reason'>): void {
    beginConnect(options, WebsocketTypes.WebSocketConnectReason.LOGIN);
  }

  testConnection(options: Omit<WebsocketTypes.TestConnectionOptions, 'reason'>): void {
    WebClient.instance.testConnect({ host: options.host, port: options.port });
  }

  register(options: Omit<WebsocketTypes.RegisterConnectOptions, 'reason'>): void {
    beginConnect(options, WebsocketTypes.WebSocketConnectReason.REGISTER);
  }

  activateAccount(options: Omit<WebsocketTypes.ActivateConnectOptions, 'reason'>): void {
    beginConnect(options, WebsocketTypes.WebSocketConnectReason.ACTIVATE_ACCOUNT);
  }

  resetPasswordRequest(options: Omit<WebsocketTypes.PasswordResetRequestConnectOptions, 'reason'>): void {
    beginConnect(options, WebsocketTypes.WebSocketConnectReason.PASSWORD_RESET_REQUEST);
  }

  resetPasswordChallenge(options: Omit<WebsocketTypes.PasswordResetChallengeConnectOptions, 'reason'>): void {
    beginConnect(options, WebsocketTypes.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE);
  }

  resetPassword(options: Omit<WebsocketTypes.PasswordResetConnectOptions, 'reason'>): void {
    beginConnect(options, WebsocketTypes.WebSocketConnectReason.PASSWORD_RESET);
  }

  disconnect(): void {
    SessionCommands.disconnect();
  }
}
