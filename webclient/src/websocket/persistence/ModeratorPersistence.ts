import { ServerDispatch } from '@app/store';
import { Data } from '@app/types';

export class ModeratorPersistence {
  static banFromServer(userName: string): void {
    ServerDispatch.banFromServer(userName);
  }

  static banHistory(userName: string, banHistory: Data.ServerInfo_Ban[]): void {
    ServerDispatch.banHistory(userName, banHistory);
  }

  static viewLogs(logs: Data.ServerInfo_ChatMessage[]): void {
    ServerDispatch.viewLogs(logs);
  }

  static warnHistory(userName: string, warnHistory: Data.ServerInfo_Warning[]): void {
    ServerDispatch.warnHistory(userName, warnHistory);
  }

  static warnListOptions(warnList: Data.Response_WarnList[]): void {
    ServerDispatch.warnListOptions(warnList);
  }

  static warnUser(userName: string): void {
    ServerDispatch.warnUser(userName);
  }

  static grantReplayAccess(replayId: number, moderatorName: string): void {
    ServerDispatch.grantReplayAccess(replayId, moderatorName);
  }

  static forceActivateUser(usernameToActivate: string, moderatorName: string): void {
    ServerDispatch.forceActivateUser(usernameToActivate, moderatorName);
  }

  static getAdminNotes(userName: string, notes: string): void {
    ServerDispatch.getAdminNotes(userName, notes);
  }

  static updateAdminNotes(userName: string, notes: string): void {
    ServerDispatch.updateAdminNotes(userName, notes);
  }
}
