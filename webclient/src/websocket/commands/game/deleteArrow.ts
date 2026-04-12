import { BackendService } from '../../services/BackendService';
import { DeleteArrowParams } from 'types';

export function deleteArrow(gameId: number, params: DeleteArrowParams): void {
  BackendService.sendGameCommand(gameId, 'Command_DeleteArrow', params);
}
