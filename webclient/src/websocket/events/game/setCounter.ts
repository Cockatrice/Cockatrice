import { GameEventMeta, SetCounterData } from 'types';
import { GamePersistence } from '../../persistence';

export function setCounter(data: SetCounterData, meta: GameEventMeta): void {
  GamePersistence.counterSet(meta.gameId, meta.playerId, data);
}
