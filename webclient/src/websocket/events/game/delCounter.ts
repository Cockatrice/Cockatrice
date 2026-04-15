import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function delCounter(data: Data.Event_DelCounter, meta: Enriched.GameEventMeta): void {
  GamePersistence.counterDeleted(meta.gameId, meta.playerId, data);
}
