import { BackendService } from '../../services/BackendService';
import { SetCardAttrParams } from 'types';

export function setCardAttr(gameId: number, params: SetCardAttrParams): void {
  BackendService.sendGameCommand(gameId, 'Command_SetCardAttr', params);
}
