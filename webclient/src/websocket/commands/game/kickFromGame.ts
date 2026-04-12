import { BackendService } from '../../services/BackendService';
import { KickFromGameParams } from 'types';

export function kickFromGame(gameId: number, params: KickFromGameParams): void {
  BackendService.sendGameCommand(gameId, 'Command_KickFromGame', params);
}
