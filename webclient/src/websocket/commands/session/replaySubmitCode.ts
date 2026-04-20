import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Command_ReplaySubmitCode_ext, Command_ReplaySubmitCodeSchema } from '@app/generated';

export function replaySubmitCode(
  replayCode: string,
  onSubmitted?: () => void,
  onFailure?: (responseCode: number) => void,
): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Command_ReplaySubmitCode_ext,
    create(Command_ReplaySubmitCodeSchema, { replayCode }),
    {
      onSuccess: onSubmitted,
      onError: onFailure,
    }
  );
}
