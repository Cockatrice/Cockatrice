import type { Event_DrawCards } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function drawCards(data: Event_DrawCards, meta: GameEventMeta): void {
  WebClient.instance.response.game.cardsDrawn(meta.gameId, meta.playerId, data);
}
