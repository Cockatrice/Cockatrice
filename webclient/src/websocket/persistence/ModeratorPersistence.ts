import { ServerDispatch } from 'store';
import type { ServerInfo_Ban } from 'generated/proto/serverinfo_ban_pb';
import type { ServerInfo_ChatMessage } from 'generated/proto/serverinfo_chat_message_pb';
import type { ServerInfo_Warning } from 'generated/proto/serverinfo_warning_pb';
import type { Response_WarnList } from 'generated/proto/response_warn_list_pb';

export class ModeratorPersistence {
  static banFromServer(userName: string): void {
    ServerDispatch.banFromServer(userName);
  }

  static banHistory(userName: string, banHistory: ServerInfo_Ban[]): void {
    ServerDispatch.banHistory(userName, banHistory);
  }

  static viewLogs(logs: ServerInfo_ChatMessage[]): void {
    ServerDispatch.viewLogs(logs);
  }

  static warnHistory(userName: string, warnHistory: ServerInfo_Warning[]): void {
    ServerDispatch.warnHistory(userName, warnHistory);
  }

  static warnListOptions(warnList: Response_WarnList[]): void {
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
