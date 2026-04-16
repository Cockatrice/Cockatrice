import type { IAdminRequest } from '@app/websocket';
import { AdminCommands } from '@app/websocket';

export class AdminRequestImpl implements IAdminRequest {
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
