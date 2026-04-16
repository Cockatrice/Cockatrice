import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function revealCards(data: Data.Event_RevealCards, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.cardsRevealed(meta.gameId, meta.playerId, data);
}
