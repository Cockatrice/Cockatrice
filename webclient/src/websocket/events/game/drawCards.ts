import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function drawCards(data: Data.Event_DrawCards, meta: Enriched.GameEventMeta): void {
  GamePersistence.cardsDrawn(meta.gameId, meta.playerId, data);
}
