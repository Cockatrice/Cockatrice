import { BackendService } from '../../services/BackendService';
import { MulliganParams } from 'types';

export function mulligan(gameId: number, params: MulliganParams): void {
  BackendService.sendGameCommand(gameId, 'Command_Mulligan', params);
}
