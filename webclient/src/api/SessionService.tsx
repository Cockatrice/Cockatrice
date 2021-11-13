import { SessionCommands } from 'websocket';

export default class SessionService {
  static addToBuddyList(userName: string) {
    SessionCommands.addToBuddyList(userName);
  }

  static removeFromBuddyList(userName: string) {
    SessionCommands.removeFromBuddyList(userName);
  }

  static addToIgnoreList(userName: string) {
    SessionCommands.addToIgnoreList(userName);
  }

  static removeFromIgnoreList(userName: string) {
    SessionCommands.removeFromIgnoreList(userName);
  }
}
