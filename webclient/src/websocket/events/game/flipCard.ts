import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function flipCard(data: Data.Event_FlipCard, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.cardFlipped(meta.gameId, meta.playerId, data);
}
