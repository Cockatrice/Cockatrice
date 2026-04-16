import { create } from '@bufbuild/protobuf';
import { Command_SetActivePhase_ext, Command_SetActivePhaseSchema, type SetActivePhaseParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function setActivePhase(gameId: number, params: SetActivePhaseParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_SetActivePhase_ext, create(Command_SetActivePhaseSchema, params));
}
