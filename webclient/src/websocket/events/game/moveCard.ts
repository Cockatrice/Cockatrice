import type { Event_MoveCard } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function moveCard(data: Event_MoveCard, meta: GameEventMeta): void {
  WebClient.instance.response.game.cardMoved(meta.gameId, meta.playerId, data);
}
