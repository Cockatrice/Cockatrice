import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function forceActivateUser(usernameToActivate: string, moderatorName: string): void {
  BackendService.sendModeratorCommand('Command_ForceActivateUser', { usernameToActivate, moderatorName }, {
    onSuccess: () => {
      ModeratorPersistence.forceActivateUser(usernameToActivate, moderatorName);
    },
  });
}
