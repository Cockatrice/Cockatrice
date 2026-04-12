import { BackendService } from '../../services/BackendService';
import { SetCardCounterParams } from 'types';

export function setCardCounter(gameId: number, params: SetCardCounterParams): void {
  BackendService.sendGameCommand(gameId, 'Command_SetCardCounter', params);
}
