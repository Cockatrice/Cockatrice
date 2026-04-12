import { BackendService } from '../../services/BackendService';
import { AttachCardParams } from 'types';

export function attachCard(gameId: number, params: AttachCardParams): void {
  BackendService.sendGameCommand(gameId, 'Command_AttachCard', params);
}
