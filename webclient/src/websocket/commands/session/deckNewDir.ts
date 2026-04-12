import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function deckNewDir(path: string, dirName: string): void {
  BackendService.sendSessionCommand('Command_DeckNewDir', { path, dirName }, {
    onSuccess: () => {
      SessionPersistence.createServerDeckDir(path, dirName);
    },
  });
}
