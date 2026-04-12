import { BackendService } from '../../services/BackendService';

export function replaySubmitCode(
  replayCode: string,
  onSuccess?: () => void,
  onError?: (responseCode: number) => void,
): void {
  BackendService.sendSessionCommand('Command_ReplaySubmitCode', { replayCode }, {
    onSuccess,
    onError,
  });
}
