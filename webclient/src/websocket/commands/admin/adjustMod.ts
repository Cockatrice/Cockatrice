import { BackendService } from '../../services/BackendService';
import { AdminPersistence } from '../../persistence';

export function adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
  BackendService.sendAdminCommand('Command_AdjustMod', { userName, shouldBeMod, shouldBeJudge }, {
    onSuccess: () => {
      AdminPersistence.adjustMod(userName, shouldBeMod, shouldBeJudge);
    },
  });
}
