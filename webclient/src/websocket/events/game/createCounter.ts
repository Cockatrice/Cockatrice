import type { Event_CreateCounter } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function createCounter(data: Event_CreateCounter, meta: GameEventMeta): void {
  WebClient.instance.response.game.counterCreated(meta.gameId, meta.playerId, data);
}
