import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_SetActivePhaseSchema, Command_SetActivePhase_ext } from 'generated/proto/command_set_active_phase_pb';
import { SetActivePhaseParams } from 'types';

export function setActivePhase(gameId: number, params: SetActivePhaseParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_SetActivePhase_ext, create(Command_SetActivePhaseSchema, params));
}
