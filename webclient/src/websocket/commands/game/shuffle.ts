import { BackendService } from '../../services/BackendService';
import { ShuffleParams } from 'types';

export function shuffle(gameId: number, params: ShuffleParams): void {
  BackendService.sendGameCommand(gameId, 'Command_Shuffle', params);
}
