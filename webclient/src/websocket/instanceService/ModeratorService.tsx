import { webClient } from "..";

export default class ModeratorService {
  static viewLogHistory(filters) {
    webClient.commands.session.viewLogHistory(filters);
  }
}