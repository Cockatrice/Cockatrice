import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function deckDelDir(path: string): void {
  BackendService.sendSessionCommand('Command_DeckDelDir', { path }, {
    onSuccess: () => {
      SessionPersistence.deleteServerDeckDir(path);
    },
  });
}
