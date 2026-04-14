import { StatusEnum, WebSocketConnectReason, WebSocketConnectOptions } from 'types';
import { SessionCommands } from 'websocket';
import { ServerInfo_User, ServerInfo_User_UserLevelFlag } from 'generated/proto/serverinfo_user_pb';

export class AuthenticationService {
  static login(options: WebSocketConnectOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.LOGIN);
  }

  static testConnection(options: WebSocketConnectOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.TEST_CONNECTION);
  }

  static register(options: WebSocketConnectOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.REGISTER);
  }

  static activateAccount(options: WebSocketConnectOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.ACTIVATE_ACCOUNT);
  }

  static resetPasswordRequest(options: WebSocketConnectOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.PASSWORD_RESET_REQUEST);
  }

  static resetPasswordChallenge(options: WebSocketConnectOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.PASSWORD_RESET_CHALLENGE);
  }

  static resetPassword(options: WebSocketConnectOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.PASSWORD_RESET);
  }

  static disconnect(): void {
    SessionCommands.disconnect();
  }

  static isConnected(state: number): boolean {
    return state === StatusEnum.LOGGED_IN;
  }

  static isModerator(user: ServerInfo_User): boolean {
    const moderatorLevel = ServerInfo_User_UserLevelFlag.IsModerator;
    // @TODO tell cockatrice not to do this so shittily
    return (user.userLevel & moderatorLevel) === moderatorLevel;
  }

  static isAdmin() {

  }
}
