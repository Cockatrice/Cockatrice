import { webClient } from "websocket";

export default class SessionService {
  static addToBuddyList(userName) {
    webClient.commands.session.addToBuddyList(userName);
  }

  static removeFromBuddyList(userName) {
    webClient.commands.session.removeFromBuddyList(userName);
  }

  static addToIgnoreList(userName) {
    webClient.commands.session.addToIgnoreList(userName);
  }

  static removeFromIgnoreList(userName) {
    webClient.commands.session.removeFromIgnoreList(userName);
  }
}