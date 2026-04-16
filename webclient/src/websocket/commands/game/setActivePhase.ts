import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function setActivePhase(gameId: number, params: Data.SetActivePhaseParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_SetActivePhase_ext, create(Data.Command_SetActivePhaseSchema, params));
}
