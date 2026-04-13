import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ConcedeSchema, Command_Concede_ext } from 'generated/proto/command_concede_pb';

export function concede(gameId: number): void {
  BackendService.sendGameCommand(gameId, Command_Concede_ext, create(Command_ConcedeSchema));
}
