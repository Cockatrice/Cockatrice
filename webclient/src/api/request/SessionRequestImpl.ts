import { SessionCommands } from '@app/websocket';
import { WebsocketTypes } from '@app/websocket/types';

export class SessionRequestImpl implements WebsocketTypes.ISessionRequest {
  addToBuddyList(userName: string): void {
    SessionCommands.addToBuddyList(userName);
  }

  removeFromBuddyList(userName: string): void {
    SessionCommands.removeFromBuddyList(userName);
  }

  addToIgnoreList(userName: string): void {
    SessionCommands.addToIgnoreList(userName);
  }

  removeFromIgnoreList(userName: string): void {
    SessionCommands.removeFromIgnoreList(userName);
  }

  changeAccountPassword(oldPassword: string, newPassword: string, hashedNewPassword?: string): void {
    SessionCommands.accountPassword(oldPassword, newPassword, hashedNewPassword);
  }

  changeAccountDetails(passwordCheck: string, realName?: string, email?: string, country?: string): void {
    SessionCommands.accountEdit(passwordCheck, realName, email, country);
  }

  changeAccountImage(image: Uint8Array): void {
    SessionCommands.accountImage(image);
  }

  sendDirectMessage(userName: string, message: string): void {
    SessionCommands.message(userName, message);
  }

  getUserInfo(userName: string): void {
    SessionCommands.getUserInfo(userName);
  }

  getUserGames(userName: string): void {
    SessionCommands.getGamesOfUser(userName);
  }

  deckDownload(deckId: number): void {
    SessionCommands.deckDownload(deckId);
  }

  replayDownload(replayId: number): void {
    SessionCommands.replayDownload(replayId);
  }
}
