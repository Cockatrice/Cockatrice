import { CreateCounterData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function createCounter(data: CreateCounterData, meta: GameEventMeta): void {
  GamePersistence.counterCreated(meta.gameId, meta.playerId, data);
}
