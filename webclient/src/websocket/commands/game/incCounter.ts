import { BackendService } from '../../services/BackendService';
import { IncCounterParams } from 'types';

export function incCounter(gameId: number, params: IncCounterParams): void {
  BackendService.sendGameCommand(gameId, 'Command_IncCounter', params);
}
