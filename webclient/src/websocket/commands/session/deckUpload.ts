import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function deckUpload(path: string, deckId: number, deckList: string): void {
  BackendService.sendSessionCommand('Command_DeckUpload', { path, deckId, deckList }, {
    responseName: 'Response_DeckUpload',
    onSuccess: (response) => {
      SessionPersistence.uploadServerDeck(path, response.newFile);
    },
  });
}
