import { BackendService } from '../../services/BackendService';
import { SetActivePhaseParams } from 'types';

export function setActivePhase(gameId: number, params: SetActivePhaseParams): void {
  BackendService.sendGameCommand(gameId, 'Command_SetActivePhase', params);
}
