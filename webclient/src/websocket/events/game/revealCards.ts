import type { Event_RevealCards } from '@app/generated';
import type { GameEventMeta } from '../../types';
import { WebClient } from '../../WebClient';

export function revealCards(data: Event_RevealCards, meta: GameEventMeta): void {
  WebClient.instance.response.game.cardsRevealed(meta.gameId, meta.playerId, data);
}
