import { BackendService } from '../../services/BackendService';
import { SetCounterParams } from 'types';

export function setCounter(gameId: number, params: SetCounterParams): void {
  BackendService.sendGameCommand(gameId, 'Command_SetCounter', params);
}
