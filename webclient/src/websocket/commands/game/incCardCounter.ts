import { BackendService } from '../../services/BackendService';
import { IncCardCounterParams } from 'types';

export function incCardCounter(gameId: number, params: IncCardCounterParams): void {
  BackendService.sendGameCommand(gameId, 'Command_IncCardCounter', params);
}
