import { BackendService } from '../../services/BackendService';

export function unconcede(gameId: number): void {
  BackendService.sendGameCommand(gameId, 'Command_Unconcede', {});
}
