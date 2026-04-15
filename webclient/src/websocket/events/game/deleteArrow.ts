import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function deleteArrow(data: Data.Event_DeleteArrow, meta: Enriched.GameEventMeta): void {
  GamePersistence.arrowDeleted(meta.gameId, meta.playerId, data);
}
