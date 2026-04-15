import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function setSideboardPlan(gameId: number, params: Data.SetSideboardPlanParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_SetSideboardPlan_ext, create(Data.Command_SetSideboardPlanSchema, params));
}
