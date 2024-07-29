import { ServerDispatch } from 'store';

export class AdminPersistence {
  static adjustMod(userName: string, shouldBeMod: boolean, shouldBeJudge: boolean) {
    console.log('adjustMod');
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
