import { BackendService } from '../../services/BackendService';
import { SetSideboardPlanParams } from 'types';

export function setSideboardPlan(gameId: number, params: SetSideboardPlanParams): void {
  BackendService.sendGameCommand(gameId, 'Command_SetSideboardPlan', params);
}
