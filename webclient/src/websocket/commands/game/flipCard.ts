import { BackendService } from '../../services/BackendService';
import { FlipCardParams } from 'types';

export function flipCard(gameId: number, params: FlipCardParams): void {
  BackendService.sendGameCommand(gameId, 'Command_FlipCard', params);
}
