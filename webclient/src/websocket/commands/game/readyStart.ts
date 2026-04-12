import { BackendService } from '../../services/BackendService';
import { ReadyStartParams } from 'types';

export function readyStart(gameId: number, params: ReadyStartParams): void {
  BackendService.sendGameCommand(gameId, 'Command_ReadyStart', params);
}
