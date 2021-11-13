import { SessionCommands } from 'websocket';

export default class ModeratorService {
  static viewLogHistory(filters): void {
    SessionCommands.viewLogHistory(filters);
  }
}
