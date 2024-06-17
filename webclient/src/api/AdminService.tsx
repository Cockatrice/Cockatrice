import { AdminCommands } from 'websocket';

export class AdminService {
  static adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
    AdminCommands.adjustMod(userName, shouldBeMod, shouldBeJudge);
  }

  static reloadConfig(): void {
    AdminCommands.reloadConfig();
  }

  static shutdownServer(reason: string, minutes: number): void {
    AdminCommands.shutdownServer(reason, minutes);
  }

  static updateServerMessage(): void {
    AdminCommands.updateServerMessage();
  }
}
