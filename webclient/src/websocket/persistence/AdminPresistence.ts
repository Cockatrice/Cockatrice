import { ServerDispatch } from 'store';

export class AdminPersistence {
  static adjustMod(userName: string, shouldBeMod: boolean, shouldBeJudge: boolean) {
    ServerDispatch.adjustMod(userName, shouldBeMod, shouldBeJudge)
  }

  static reloadConfig() {
    ServerDispatch.reloadConfig();
  }

  static shutdownServer() {
    ServerDispatch.shutdownServer();
  }

  static updateServerMessage() {
    ServerDispatch.updateServerMessage();
  }
}
