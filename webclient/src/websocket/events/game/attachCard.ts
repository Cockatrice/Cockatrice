import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function attachCard(data: Data.Event_AttachCard, meta: Enriched.GameEventMeta): void {
  GamePersistence.cardAttached(meta.gameId, meta.playerId, data);
}
