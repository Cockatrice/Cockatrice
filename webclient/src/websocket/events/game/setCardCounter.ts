import type { Event_SetCardCounter } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function setCardCounter(data: Event_SetCardCounter, meta: GameEventMeta): void {
  WebClient.instance.response.game.cardCounterChanged(meta.gameId, meta.playerId, data);
}
