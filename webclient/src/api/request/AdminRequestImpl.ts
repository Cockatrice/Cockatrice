import { AdminCommands } from '@app/websocket';
import { WebsocketTypes } from '@app/websocket/types';

export class AdminRequestImpl implements WebsocketTypes.IAdminRequest {
  adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
    AdminCommands.adjustMod(userName, shouldBeMod, shouldBeJudge);
  }

  reloadConfig(): void {
    AdminCommands.reloadConfig();
  }

  shutdownServer(reason: string, minutes: number): void {
    AdminCommands.shutdownServer(reason, minutes);
  }

  updateServerMessage(): void {
    AdminCommands.updateServerMessage();
  }
}
