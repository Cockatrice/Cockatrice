import { webClient } from "websocket";

export default class ModeratorService {
  static viewLogHistory(filters) {
    webClient.commands.session.viewLogHistory(filters);
  }
}