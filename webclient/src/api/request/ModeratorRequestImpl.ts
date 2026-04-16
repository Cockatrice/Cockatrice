import { Data } from '@app/types';
import type { IModeratorRequest } from '@app/websocket';
import { ModeratorCommands } from '@app/websocket';

export class ModeratorRequestImpl implements IModeratorRequest {
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

  getBanHistory(userName: string): void {
    ModeratorCommands.getBanHistory(userName);
  }

  getWarnHistory(userName: string): void {
    ModeratorCommands.getWarnHistory(userName);
  }

  getWarnList(modName: string, userName: string, userClientid: string): void {
    ModeratorCommands.getWarnList(modName, userName, userClientid);
  }

  viewLogHistory(filters: Data.ViewLogHistoryParams): void {
    ModeratorCommands.viewLogHistory(filters);
  }

  warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void {
    ModeratorCommands.warnUser(userName, reason, clientid, removeMessages);
  }
}
