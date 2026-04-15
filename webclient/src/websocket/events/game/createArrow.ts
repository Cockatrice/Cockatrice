import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function createArrow(data: Data.Event_CreateArrow, meta: Enriched.GameEventMeta): void {
  GamePersistence.arrowCreated(meta.gameId, meta.playerId, data);
}
