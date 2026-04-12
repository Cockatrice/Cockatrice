import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function deckDel(deckId: number): void {
  BackendService.sendSessionCommand('Command_DeckDel', { deckId }, {
    onSuccess: () => {
      SessionPersistence.deleteServerDeck(deckId);
    },
  });
}
