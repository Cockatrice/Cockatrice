import { GameEventMeta, SetCardAttrData } from 'types';
import { GamePersistence } from '../../persistence';

export function setCardAttr(data: SetCardAttrData, meta: GameEventMeta): void {
  GamePersistence.cardAttrChanged(meta.gameId, meta.playerId, data);
}
