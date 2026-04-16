import { App, Enriched } from '@app/types';
import type { IAuthenticationRequest } from '@app/websocket';
import { SessionCommands } from '@app/websocket';

export class AuthenticationRequestImpl implements IAuthenticationRequest {
  login(options: Omit<Enriched.LoginConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.LOGIN });
  }

  testConnection(options: Omit<Enriched.TestConnectionOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.TEST_CONNECTION });
  }

  register(options: Omit<Enriched.RegisterConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.REGISTER });
  }

  activateAccount(options: Omit<Enriched.ActivateConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT });
  }

  resetPasswordRequest(options: Omit<Enriched.PasswordResetRequestConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET_REQUEST });
  }

  resetPasswordChallenge(options: Omit<Enriched.PasswordResetChallengeConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE });
  }

  resetPassword(options: Omit<Enriched.PasswordResetConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET });
  }

  disconnect(): void {
    SessionCommands.disconnect();
  }
}
