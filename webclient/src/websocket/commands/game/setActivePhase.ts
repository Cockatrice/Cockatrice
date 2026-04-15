import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function setActivePhase(gameId: number, params: Data.SetActivePhaseParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_SetActivePhase_ext, create(Data.Command_SetActivePhaseSchema, params));
}
