import { BackendService } from '../../services/BackendService';

export function replayGetCode(gameId: number, onCodeReceived: (code: string) => void): void {
  BackendService.sendSessionCommand('Command_ReplayGetCode', { gameId }, {
    responseName: 'Response_ReplayGetCode',
    onSuccess: (response) => {
      onCodeReceived(response.replayCode);
    },
  });
}
