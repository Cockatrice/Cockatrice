import { SessionCommands } from "websocket";

export default class ModeratorService {
  static viewLogHistory(filters) {
    SessionCommands.viewLogHistory(filters);
  }
}
