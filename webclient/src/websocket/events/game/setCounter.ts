import type { Event_SetCounter } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function setCounter(data: Event_SetCounter, meta: GameEventMeta): void {
  WebClient.instance.response.game.counterSet(meta.gameId, meta.playerId, data);
}
