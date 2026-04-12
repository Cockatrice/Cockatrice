import { BackendService } from '../../services/BackendService';
import { DeckSelectParams } from 'types';

export function deckSelect(gameId: number, params: DeckSelectParams): void {
  BackendService.sendGameCommand(gameId, 'Command_DeckSelect', params);
}
