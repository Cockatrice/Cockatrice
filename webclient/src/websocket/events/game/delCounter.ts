import type { Event_DelCounter } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function delCounter(data: Event_DelCounter, meta: GameEventMeta): void {
  WebClient.instance.response.game.counterDeleted(meta.gameId, meta.playerId, data);
}
