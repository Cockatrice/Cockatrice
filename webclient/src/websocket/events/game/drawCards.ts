import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function drawCards(data: Data.Event_DrawCards, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.cardsDrawn(meta.gameId, meta.playerId, data);
}
