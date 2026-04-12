import { BackendService } from '../../services/BackendService';
import { CreateArrowParams } from 'types';

export function createArrow(gameId: number, params: CreateArrowParams): void {
  BackendService.sendGameCommand(gameId, 'Command_CreateArrow', params);
}
