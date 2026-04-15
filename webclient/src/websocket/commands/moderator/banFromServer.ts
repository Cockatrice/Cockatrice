import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function banFromServer(minutes: number, userName?: string, address?: string, reason?: string,
  visibleReason?: string, clientid?: string, removeMessages?: number): void {
  BackendService.sendModeratorCommand('Command_BanFromServer', {
    minutes, userName, address, reason, visibleReason, clientid, removeMessages
  }, {
    onSuccess: () => {
      ModeratorPersistence.banFromServer(userName);
    },
  });
}
