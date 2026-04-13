import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_SetActivePhaseSchema, Command_SetActivePhase_ext } from 'generated/proto/command_set_active_phase_pb';
import { SetActivePhaseParams } from 'types';

export function setActivePhase(gameId: number, params: SetActivePhaseParams): void {
  BackendService.sendGameCommand(gameId, Command_SetActivePhase_ext, create(Command_SetActivePhaseSchema, params));
}
