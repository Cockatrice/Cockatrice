import { BackendService } from '../../services/BackendService';

export function reverseTurn(gameId: number): void {
  BackendService.sendGameCommand(gameId, 'Command_ReverseTurn', {});
}
