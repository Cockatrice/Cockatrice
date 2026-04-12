import { BackendService } from '../../services/BackendService';
import { DelCounterParams } from 'types';

export function delCounter(gameId: number, params: DelCounterParams): void {
  BackendService.sendGameCommand(gameId, 'Command_DelCounter', params);
}
