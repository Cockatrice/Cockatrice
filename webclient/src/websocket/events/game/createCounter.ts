import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function createCounter(data: Data.Event_CreateCounter, meta: Enriched.GameEventMeta): void {
  GamePersistence.counterCreated(meta.gameId, meta.playerId, data);
}
