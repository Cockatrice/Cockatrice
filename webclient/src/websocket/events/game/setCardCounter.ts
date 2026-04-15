import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function setCardCounter(data: Data.Event_SetCardCounter, meta: Enriched.GameEventMeta): void {
  GamePersistence.cardCounterChanged(meta.gameId, meta.playerId, data);
}
