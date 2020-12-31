import { StatusEnum } from "types";
import webClient from "WebClient/WebClient";

export class AuthenticationService {
  static connect(options) {
    webClient.services.session.connectServer(options);
  }
  static disconnect() {
    webClient.services.session.disconnectServer();
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