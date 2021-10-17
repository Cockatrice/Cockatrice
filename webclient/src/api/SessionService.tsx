import { SessionCommands } from "websocket";

export default class SessionService {
  static addToBuddyList(userName) {
    SessionCommands.addToBuddyList(userName);
  }

  static removeFromBuddyList(userName) {
    SessionCommands.removeFromBuddyList(userName);
  }

  static addToIgnoreList(userName) {
    SessionCommands.addToIgnoreList(userName);
  }

  static removeFromIgnoreList(userName) {
    SessionCommands.removeFromIgnoreList(userName);
  }
}