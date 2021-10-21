import {StatusEnum, User} from "types";
import {SessionCommands, webClient} from "websocket";
import {WebSocketConnectReason, WebSocketOptions} from "../websocket/services/WebSocketService";

export default class AuthenticationService {
  static connect(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.LOGIN);
  }

  static register(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.REGISTER);
  }

  static registerWithActivationCode(options: WebSocketOptions): void {
    SessionCommands.connect(options, WebSocketConnectReason.ACTIVATE_ACCOUNT);
  }

  static disconnect(): void {
    SessionCommands.disconnect();
  }

  static isConnected(state: number): boolean {
    return state === StatusEnum.LOGGEDIN;
  }

  static isModerator(user: User): boolean {
    const moderatorLevel = webClient.protobuf.controller.ServerInfo_User.UserLevelFlag.IsModerator;
    // @TODO tell cockatrice not to do this so shittily
    return (user.userLevel & moderatorLevel) === moderatorLevel;
  }

  static isAdmin() {

  }
}
