import { ServerConnectParams } from "store";
import { StatusEnum } from "types";
import { webClient, SessionCommands } from "websocket";

export default class AuthenticationService {
  static connect(options: ServerConnectParams) {
    SessionCommands.connect(options);
  }
  static disconnect() {
    SessionCommands.disconnect();
  }

  static isConnected(state) {
    return state === StatusEnum.LOGGEDIN;
  }

  static isModerator(user) {
    const moderatorLevel = webClient.protobuf.controller.ServerInfo_User.UserLevelFlag.IsModerator;
    // @TODO tell cockatrice not to do this so shittily
    return (user.userLevel & moderatorLevel) === moderatorLevel;
  }

  static isAdmin() {

  }
}
