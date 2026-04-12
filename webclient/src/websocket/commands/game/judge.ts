import { BackendService } from '../../services/BackendService';

export function judge(gameId: number, targetId: number, innerGameCommand: any): void {
  BackendService.sendGameCommand(gameId, 'Command_Judge', {
    targetId,
    gameCommand: [innerGameCommand],
  });
}
