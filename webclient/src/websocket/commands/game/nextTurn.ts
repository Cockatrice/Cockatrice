import { BackendService } from '../../services/BackendService';

export function nextTurn(gameId: number): void {
  BackendService.sendGameCommand(gameId, 'Command_NextTurn', {});
}
