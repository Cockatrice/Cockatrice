import { BackendService } from '../../services/BackendService';

export function undoDraw(gameId: number): void {
  BackendService.sendGameCommand(gameId, 'Command_UndoDraw', {});
}
