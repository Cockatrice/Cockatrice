import { BackendService } from '../../services/BackendService';
import { DrawCardsParams } from 'types';

export function drawCards(gameId: number, params: DrawCardsParams): void {
  BackendService.sendGameCommand(gameId, 'Command_DrawCards', params);
}
