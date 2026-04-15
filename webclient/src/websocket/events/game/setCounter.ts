import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function setCounter(data: Data.Event_SetCounter, meta: Enriched.GameEventMeta): void {
  GamePersistence.counterSet(meta.gameId, meta.playerId, data);
}
