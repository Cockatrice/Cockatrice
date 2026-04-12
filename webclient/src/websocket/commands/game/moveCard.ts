import { BackendService } from '../../services/BackendService';
import { MoveCardParams } from 'types';

export function moveCard(gameId: number, params: MoveCardParams): void {
  BackendService.sendGameCommand(gameId, 'Command_MoveCard', params);
}
