import { Data } from '@app/types';
import { ModeratorCommands } from '@app/websocket';
import { WebsocketTypes } from '@app/websocket/types';

export class ModeratorRequestImpl implements WebsocketTypes.IModeratorRequest {
  banFromServer(
    minutes: number,
    userName?: string,
    address?: string,
    reason?: string,
    visibleReason?: string,
    clientid?: string,
    removeMessages?: number
  ): void {
    ModeratorCommands.banFromServer(minutes, userName, address, reason, visibleReason, clientid, removeMessages);
  }

  forceActivateUser(usernameToActivate: string, moderatorName: string): void {
    ModeratorCommands.forceActivateUser(usernameToActivate, moderatorName);
  }

  getAdminNotes(userName: string): void {
    ModeratorCommands.getAdminNotes(userName);
  }

  getBanHistory(userName: string): void {
    ModeratorCommands.getBanHistory(userName);
  }

  getWarnHistory(userName: string): void {
    ModeratorCommands.getWarnHistory(userName);
  }

  getWarnList(modName: string, userName: string, userClientid: string): void {
    ModeratorCommands.getWarnList(modName, userName, userClientid);
  }

  grantReplayAccess(replayId: number, moderatorName: string): void {
    ModeratorCommands.grantReplayAccess(replayId, moderatorName);
  }

  updateAdminNotes(userName: string, notes: string): void {
    ModeratorCommands.updateAdminNotes(userName, notes);
  }

  viewLogHistory(filters: Data.ViewLogHistoryParams): void {
    ModeratorCommands.viewLogHistory(filters);
  }

  warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void {
    ModeratorCommands.warnUser(userName, reason, clientid, removeMessages);
  }
}
