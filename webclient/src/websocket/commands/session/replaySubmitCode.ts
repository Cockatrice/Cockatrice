import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ReplaySubmitCodeSchema, Command_ReplaySubmitCode_ext } from 'generated/proto/command_replay_submit_code_pb';

export function replaySubmitCode(
  replayCode: string,
  onSuccess?: () => void,
  onError?: (responseCode: number) => void,
): void {
  webClient.protobuf.sendSessionCommand(Command_ReplaySubmitCode_ext, create(Command_ReplaySubmitCodeSchema, { replayCode }), {
    onSuccess,
    onError,
  });
}
