import { create } from '@bufbuild/protobuf';
import { Command_SetSideboardPlan_ext, Command_SetSideboardPlanSchema, type SetSideboardPlanParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function setSideboardPlan(gameId: number, params: SetSideboardPlanParams): void {
  WebClient.instance.protobuf.sendGameCommand(
    gameId,
    Command_SetSideboardPlan_ext,
    create(Command_SetSideboardPlanSchema, params)
  );
}
