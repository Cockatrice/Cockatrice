import { DelCounterData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function delCounter(data: DelCounterData, meta: GameEventMeta): void {
  GamePersistence.counterDeleted(meta.gameId, meta.playerId, data);
}
