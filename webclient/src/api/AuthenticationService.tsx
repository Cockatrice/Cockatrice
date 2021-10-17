import { ServerConnectParams } from "store";
import { StatusEnum, User} from "types";
import { webClient, SessionCommands } from "websocket";

export default class AuthenticationService {
  static connect(options: ServerConnectParams): void {
    SessionCommands.connect(options);
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
