import { BackendService } from '../../services/BackendService';

export function concede(gameId: number): void {
  BackendService.sendGameCommand(gameId, 'Command_Concede', {});
}
