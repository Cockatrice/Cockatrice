import { BackendService } from '../../services/BackendService';
import { RevealCardsParams } from 'types';

export function revealCards(gameId: number, params: RevealCardsParams): void {
  BackendService.sendGameCommand(gameId, 'Command_RevealCards', params);
}
