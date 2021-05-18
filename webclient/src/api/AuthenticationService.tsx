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
    return user.userLevel >= webClient.pb.ServerInfo_User.UserLevelFlag.IsModerator;
  }

  static isAdmin() {

  }
}