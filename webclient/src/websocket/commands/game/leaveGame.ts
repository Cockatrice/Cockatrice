import { BackendService } from '../../services/BackendService';

export function leaveGame(gameId: number): void {
  BackendService.sendGameCommand(gameId, 'Command_LeaveGame', {});
}
