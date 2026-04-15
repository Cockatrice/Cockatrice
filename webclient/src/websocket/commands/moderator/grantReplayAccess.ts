import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function grantReplayAccess(replayId: number, moderatorName: string): void {
  BackendService.sendModeratorCommand('Command_GrantReplayAccess', { replayId, moderatorName }, {
    onSuccess: () => {
      ModeratorPersistence.grantReplayAccess(replayId, moderatorName);
    },
  });
}
