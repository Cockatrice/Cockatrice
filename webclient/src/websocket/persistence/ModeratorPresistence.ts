import { ServerDispatch } from 'store';
import { BanHistoryItem, LogItem, WarnHistoryItem, WarnListItem } from 'types';

import NormalizeService from '../utils/NormalizeService';

export class ModeratorPersistence {
  static banFromServer(userName: string): void {
    console.log(userName);
  }

  static banHistory(userName: string, banHistory: BanHistoryItem[]): void {
    console.log(userName, banHistory);
  }

  static viewLogs(logs: LogItem[]): void {
    ServerDispatch.viewLogs(NormalizeService.normalizeLogs(logs));
  }

  static warnHistory(userName: string, warnHistory: WarnHistoryItem[]): void {
    console.log(userName, warnHistory);
  }

  static warnList(warnList: WarnListItem[]): void {
    console.log(warnList);
  }

  static warnUser(userName: string): void {
    console.log(userName);
  }
}
