import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function setSideboardPlan(gameId: number, params: Data.SetSideboardPlanParams): void {
  WebClient.instance.protobuf.sendGameCommand(
    gameId,
    Data.Command_SetSideboardPlan_ext,
    create(Data.Command_SetSideboardPlanSchema, params)
  );
}
