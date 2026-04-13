import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ReplayModifyMatchSchema, Command_ReplayModifyMatch_ext } from 'generated/proto/command_replay_modify_match_pb';
import { SessionPersistence } from '../../persistence';

export function replayModifyMatch(gameId: number, doNotHide: boolean): void {
  BackendService.sendSessionCommand(Command_ReplayModifyMatch_ext, create(Command_ReplayModifyMatchSchema, { gameId, doNotHide }), {
    onSuccess: () => {
      SessionPersistence.replayModifyMatch(gameId, doNotHide);
    },
  });
}
