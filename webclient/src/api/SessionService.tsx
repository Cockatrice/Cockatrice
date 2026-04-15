import { SessionCommands } from 'websocket';

export class SessionService {
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

  static changeAccountPassword(oldPassword: string, newPassword: string, hashedNewPassword?: string): void {
    SessionCommands.accountPassword(oldPassword, newPassword, hashedNewPassword);
  }

  static changeAccountDetails(passwordCheck: string, realName?: string, email?: string, country?: string): void {
    SessionCommands.accountEdit(passwordCheck, realName, email, country);
  }

  static changeAccountImage(image: Uint8Array): void {
    SessionCommands.accountImage(image);
  }

  static sendDirectMessage(userName: string, message: string): void {
    SessionCommands.message(userName, message);
  }

  static getUserInfo(userName: string): void {
    SessionCommands.getUserInfo(userName);
  }

  static getUserGames(userName: string): void {
    SessionCommands.getGamesOfUser(userName);
  }
}
