import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ReplayDeleteMatchSchema, Command_ReplayDeleteMatch_ext } from 'generated/proto/command_replay_delete_match_pb';
import { SessionPersistence } from '../../persistence';

export function replayDeleteMatch(gameId: number): void {
  BackendService.sendSessionCommand(Command_ReplayDeleteMatch_ext, create(Command_ReplayDeleteMatchSchema, { gameId }), {
    onSuccess: () => {
      SessionPersistence.replayDeleteMatch(gameId);
    },
  });
}
