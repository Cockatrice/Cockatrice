import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void {
  BackendService.sendModeratorCommand('Command_WarnUser', { userName, reason, clientid, removeMessages }, {
    onSuccess: () => {
      ModeratorPersistence.warnUser(userName);
    },
  });
}
