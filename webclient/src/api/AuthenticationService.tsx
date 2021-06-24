import { StatusEnum } from "types";
import { webClient } from "websocket";

export default class AuthenticationService {
  static connect(options) {
    webClient.persistence.session.connectServer(options);
  }
  static disconnect() {
    webClient.persistence.session.disconnectServer();
  }

  static isConnected(state) {
    return state === StatusEnum.LOGGEDIN;
  }

  static isModerator(user) {
    const moderatorLevel = webClient.pb.ServerInfo_User.UserLevelFlag.IsModerator;
    // @TODO tell cockatrice not to do this so shittily
    return (user.userLevel & moderatorLevel) === moderatorLevel;
  }

  static isAdmin() {

  }
}