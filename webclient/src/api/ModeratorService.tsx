import { ModeratorCommands } from 'websocket';

export default class ModeratorService {
  static viewLogHistory(filters): void {
    ModeratorCommands.viewLogHistory(filters);
  }
}
