import { BackendService } from '../../services/BackendService';
import { CreateCounterParams } from 'types';

export function createCounter(gameId: number, params: CreateCounterParams): void {
  BackendService.sendGameCommand(gameId, 'Command_CreateCounter', params);
}
