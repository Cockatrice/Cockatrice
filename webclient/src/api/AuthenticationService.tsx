import { App, Data, Enriched } from '@app/types';
import { SessionCommands } from '@app/websocket';

export class AuthenticationService {
  static login(options: Omit<Enriched.LoginConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.LOGIN });
  }

  static testConnection(options: Omit<Enriched.TestConnectionOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.TEST_CONNECTION });
  }

  static register(options: Omit<Enriched.RegisterConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.REGISTER });
  }

  static activateAccount(options: Omit<Enriched.ActivateConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.ACTIVATE_ACCOUNT });
  }

  static resetPasswordRequest(options: Omit<Enriched.PasswordResetRequestConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET_REQUEST });
  }

  static resetPasswordChallenge(options: Omit<Enriched.PasswordResetChallengeConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE });
  }

  static resetPassword(options: Omit<Enriched.PasswordResetConnectOptions, 'reason'>): void {
    SessionCommands.connect({ ...options, reason: App.WebSocketConnectReason.PASSWORD_RESET });
  }

  static disconnect(): void {
    SessionCommands.disconnect();
  }

  static isConnected(state: number): boolean {
    return state === App.StatusEnum.LOGGED_IN;
  }

  static isModerator(user: Data.ServerInfo_User): boolean {
    const moderatorLevel = Data.ServerInfo_User_UserLevelFlag.IsModerator;
    // @TODO tell cockatrice not to do this so shittily
    return (user.userLevel & moderatorLevel) === moderatorLevel;
  }

  static isAdmin() {

  }
}
