import type { Event_FlipCard } from '@app/generated';
import type { GameEventMeta } from '../../types';
import { WebClient } from '../../WebClient';

export function flipCard(data: Event_FlipCard, meta: GameEventMeta): void {
  WebClient.instance.response.game.cardFlipped(meta.gameId, meta.playerId, data);
}
