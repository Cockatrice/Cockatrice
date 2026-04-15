import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function revealCards(data: Data.Event_RevealCards, meta: Enriched.GameEventMeta): void {
  GamePersistence.cardsRevealed(meta.gameId, meta.playerId, data);
}
