import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function replayList(): void {
  BackendService.sendSessionCommand('Command_ReplayList', {}, {
    responseName: 'Response_ReplayList',
    onSuccess: (response) => {
      SessionPersistence.replayList(response.matchList);
    },
  });
}
