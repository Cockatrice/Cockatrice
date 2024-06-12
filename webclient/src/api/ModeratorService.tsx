import { ModeratorCommands } from 'websocket';

export default class ModeratorService {
  static viewLogHistory(filters): void {
    ModeratorCommands.viewLogHistory(filters);
  }

  static banFromServer(minutes: number, userName?: string, address?: string, reason?: string, visibleReason?: string, clientid?: string, removeMessages?: number): void {
    ModeratorCommands.banFromServer(minutes, userName, address, reason, visibleReason, clientid, removeMessages);
  }

  static getBanHistory(userName: string): void {
    ModeratorCommands.getBanHistory(userName);
  }

  static getWarnHistory(userName: string): void {
    ModeratorCommands.getWarnHistory(userName);
  }

  static warnUser(userName: string, reason: string, clientid?: string, removeMessage?: boolean): void {
    ModeratorCommands.warnUser(userName, reason, clientid, removeMessage);
  }

  static getWarnList(modName: string, userName: string, userClientid: string): void {
    ModeratorCommands.getWarnList(modName, userName, userClientid);
  }
}
