import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_SetSideboardPlanSchema, Command_SetSideboardPlan_ext } from 'generated/proto/command_set_sideboard_plan_pb';
import { SetSideboardPlanParams } from 'types';

export function setSideboardPlan(gameId: number, params: SetSideboardPlanParams): void {
  BackendService.sendGameCommand(gameId, Command_SetSideboardPlan_ext, create(Command_SetSideboardPlanSchema, params));
}
