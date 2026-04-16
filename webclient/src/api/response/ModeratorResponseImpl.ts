import { Data } from '@app/types';
import type { IModeratorResponse } from '@app/websocket';
import { ServerDispatch } from '@app/store';

export class ModeratorResponseImpl implements IModeratorResponse {
  banFromServer(userName: string): void {
    ServerDispatch.banFromServer(userName);
  }

  banHistory(userName: string, banHistory: Data.ServerInfo_Ban[]): void {
    ServerDispatch.banHistory(userName, banHistory);
  }

  viewLogs(logs: Data.ServerInfo_ChatMessage[]): void {
    ServerDispatch.viewLogs(logs);
  }

  warnHistory(userName: string, warnHistory: Data.ServerInfo_Warning[]): void {
    ServerDispatch.warnHistory(userName, warnHistory);
  }

  warnListOptions(warnList: Data.Response_WarnList[]): void {
    ServerDispatch.warnListOptions(warnList);
  }

  warnUser(userName: string): void {
    ServerDispatch.warnUser(userName);
  }

  grantReplayAccess(replayId: number, moderatorName: string): void {
    ServerDispatch.grantReplayAccess(replayId, moderatorName);
  }

  forceActivateUser(usernameToActivate: string, moderatorName: string): void {
    ServerDispatch.forceActivateUser(usernameToActivate, moderatorName);
  }

  getAdminNotes(userName: string, notes: string): void {
    ServerDispatch.getAdminNotes(userName, notes);
  }

  updateAdminNotes(userName: string, notes: string): void {
    ServerDispatch.updateAdminNotes(userName, notes);
  }
}
