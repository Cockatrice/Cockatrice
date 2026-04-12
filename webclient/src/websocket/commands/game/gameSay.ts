import { BackendService } from '../../services/BackendService';
import { GameSayParams } from 'types';

export function gameSay(gameId: number, params: GameSayParams): void {
  BackendService.sendGameCommand(gameId, 'Command_GameSay', params);
}
