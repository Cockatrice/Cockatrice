import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function message(userName: string, message: string): void {
  BackendService.sendSessionCommand('Command_Message', { userName, message }, {
    onSuccess: () => {
      SessionPersistence.directMessageSent(userName, message);
    },
  });
}
