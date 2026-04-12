import { GameEventMeta, SetCardCounterData } from 'types';
import { GamePersistence } from '../../persistence';

export function setCardCounter(data: SetCardCounterData, meta: GameEventMeta): void {
  GamePersistence.cardCounterChanged(meta.gameId, meta.playerId, data);
}
