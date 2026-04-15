import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function replaySubmitCode(
  replayCode: string,
  onSuccess?: () => void,
  onError?: (responseCode: number) => void,
): void {
  webClient.protobuf.sendSessionCommand(Data.Command_ReplaySubmitCode_ext, create(Data.Command_ReplaySubmitCodeSchema, { replayCode }), {
    onSuccess,
    onError,
  });
}
