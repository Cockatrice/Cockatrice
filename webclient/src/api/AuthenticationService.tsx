import { StatusEnum, User, WebSocketConnectReason, WebSocketOptions } from 'types';
import { SessionCommands, webClient } from 'websocket';

export default class AuthenticationService {
  static login(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.LOGIN);
  }

  static register(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.REGISTER);
  }

  static activateAccount(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.ACTIVATE_ACCOUNT);
  }

  static resetPasswordRequest(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.PASSWORD_RESET_REQUEST);
  }

  static resetPasswordChallenge(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.PASSWORD_RESET_CHALLENGE);
  }

  static resetPassword(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.PASSWORD_RESET);
  }

  static disconnect(): void {
    SessionCommands.disconnect();
  }

  static isConnected(state: number): boolean {
    return state === StatusEnum.LOGGED_IN;
  }

  static isModerator(user: User): boolean {
    const moderatorLevel = webClient.protobuf.controller.ServerInfo_User.UserLevelFlag.IsModerator;
    // @TODO tell cockatrice not to do this so shittily
    return (user.userLevel & moderatorLevel) === moderatorLevel;
  }

  static isAdmin() {

  }

  static connectionAttemptMade() {
    return webClient.connectionAttemptMade;
  }
}
