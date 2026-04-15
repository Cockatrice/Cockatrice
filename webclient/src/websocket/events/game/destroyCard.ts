import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function destroyCard(data: Data.Event_DestroyCard, meta: Enriched.GameEventMeta): void {
  GamePersistence.cardDestroyed(meta.gameId, meta.playerId, data);
}
