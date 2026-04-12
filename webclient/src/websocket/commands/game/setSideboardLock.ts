import { BackendService } from '../../services/BackendService';
import { SetSideboardLockParams } from 'types';

export function setSideboardLock(gameId: number, params: SetSideboardLockParams): void {
  BackendService.sendGameCommand(gameId, 'Command_SetSideboardLock', params);
}
