import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function deckList(): void {
  BackendService.sendSessionCommand('Command_DeckList', {}, {
    responseName: 'Response_DeckList',
    onSuccess: (response) => {
      SessionPersistence.updateServerDecks(response);
    },
  });
}
