import webClient from "WebClient/WebClient";

export class ModeratorService {
  static viewLogHistory(filters) {
    webClient.commands.session.viewLogHistory(filters);
  }
}