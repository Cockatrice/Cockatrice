import { ServerDispatch } from 'store';
import { BanHistoryItem, LogItem, WarnHistoryItem, WarnListItem } from 'types';

import NormalizeService from '../utils/NormalizeService';

export class ModeratorPersistence {
  static banFromServer(userName: string): void {
    ServerDispatch.banFromServer(userName);
  }

  static banHistory(userName: string, banHistory: BanHistoryItem[]): void {
    ServerDispatch.banHistory(userName, banHistory);
  }

  static viewLogs(logs: LogItem[]): void {
    ServerDispatch.viewLogs(NormalizeService.normalizeLogs(logs));
  }

  static warnHistory(userName: string, warnHistory: WarnHistoryItem[]): void {
    ServerDispatch.warnHistory(userName, warnHistory);
  }

  static warnListOptions(warnList: WarnListItem[]): void {
    ServerDispatch.warnListOptions(warnList);
  }

  static warnUser(userName: string): void {
    ServerDispatch.warnUser(userName);
  }
}
