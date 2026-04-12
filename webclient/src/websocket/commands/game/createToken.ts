import { BackendService } from '../../services/BackendService';
import { CreateTokenParams } from 'types';

export function createToken(gameId: number, params: CreateTokenParams): void {
  BackendService.sendGameCommand(gameId, 'Command_CreateToken', params);
}
