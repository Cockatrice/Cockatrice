import webClient from 'WebClient/WebClient';

export class ModeratorService {
  static viewLogHistory() {
    webClient.commands.session.viewLogHistory()
  }
}