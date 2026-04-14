import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_SetSideboardPlanSchema, Command_SetSideboardPlan_ext } from 'generated/proto/command_set_sideboard_plan_pb';
import { SetSideboardPlanParams } from 'types';

export function setSideboardPlan(gameId: number, params: SetSideboardPlanParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_SetSideboardPlan_ext, create(Command_SetSideboardPlanSchema, params));
}
