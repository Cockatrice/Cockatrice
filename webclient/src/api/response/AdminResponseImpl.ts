import { WebsocketTypes } from '@app/websocket/types';
import { ServerDispatch } from '@app/store';

export class AdminResponseImpl implements WebsocketTypes.IAdminResponse {
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
