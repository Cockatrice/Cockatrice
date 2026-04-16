import { App, Enriched } from '@app/types';
import { WebClient, StatusEnum, SessionCommands } from '@app/websocket';
import type { IAuthenticationRequest, AuthRequestMap } from '@app/websocket';

import { setPendingOptions } from '../connectionState';

interface AppAuthRequestOverrides extends AuthRequestMap {
  LoginParams: Omit<Enriched.LoginConnectOptions, 'reason'>;
  ConnectTarget: Omit<Enriched.TestConnectionOptions, 'reason'>;
  RegisterParams: Omit<Enriched.RegisterConnectOptions, 'reason'>;
  ActivateParams: Omit<Enriched.ActivateConnectOptions, 'reason'>;
  ForgotPasswordRequestParams: Omit<Enriched.PasswordResetRequestConnectOptions, 'reason'>;
  ForgotPasswordChallengeParams: Omit<Enriched.PasswordResetChallengeConnectOptions, 'reason'>;
  ForgotPasswordResetParams: Omit<Enriched.PasswordResetConnectOptions, 'reason'>;
}

export class AuthenticationRequestImpl implements IAuthenticationRequest<AppAuthRequestOverrides> {
  login(options: Omit<Enriched.LoginConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: App.WebSocketConnectReason.LOGIN });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  testConnection(options: Omit<Enriched.TestConnectionOptions, 'reason'>): void {
    WebClient.instance.testConnect({ host: options.host, port: options.port });
  }

  register(options: Omit<Enriched.RegisterConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: App.WebSocketConnectReason.REGISTER });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  activateAccount(options: Omit<Enriched.ActivateConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPasswordRequest(options: Omit<Enriched.PasswordResetRequestConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET_REQUEST });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPasswordChallenge(options: Omit<Enriched.PasswordResetChallengeConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  resetPassword(options: Omit<Enriched.PasswordResetConnectOptions, 'reason'>): void {
    setPendingOptions({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET });
    SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
    WebClient.instance.connect({ host: options.host, port: options.port });
  }

  disconnect(): void {
    SessionCommands.disconnect();
  }
}
