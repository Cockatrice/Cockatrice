import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function flipCard(data: Data.Event_FlipCard, meta: Enriched.GameEventMeta): void {
  GamePersistence.cardFlipped(meta.gameId, meta.playerId, data);
}
