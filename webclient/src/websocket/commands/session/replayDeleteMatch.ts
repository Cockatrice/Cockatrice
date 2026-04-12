import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function replayDeleteMatch(gameId: number): void {
  BackendService.sendSessionCommand('Command_ReplayDeleteMatch', { gameId }, {
    onSuccess: () => {
      SessionPersistence.replayDeleteMatch(gameId);
    },
  });
}
