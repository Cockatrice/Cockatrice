import type { IAdminResponse } from '@app/websocket';
import { ServerDispatch } from '@app/store';

export class AdminResponseImpl implements IAdminResponse {
  adjustMod(userName: string, shouldBeMod: boolean, shouldBeJudge: boolean): void {
    ServerDispatch.adjustMod(userName, shouldBeMod, shouldBeJudge);
  }

  reloadConfig(): void {
    ServerDispatch.reloadConfig();
  }

  shutdownServer(): void {
    ServerDispatch.shutdownServer();
  }

  updateServerMessage(): void {
    ServerDispatch.updateServerMessage();
  }
}
