import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function replayModifyMatch(gameId: number, doNotHide: boolean): void {
  BackendService.sendSessionCommand('Command_ReplayModifyMatch', { gameId, doNotHide }, {
    onSuccess: () => {
      SessionPersistence.replayModifyMatch(gameId, doNotHide);
    },
  });
}
