import type { Event_DestroyCard } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function destroyCard(data: Event_DestroyCard, meta: GameEventMeta): void {
  WebClient.instance.response.game.cardDestroyed(meta.gameId, meta.playerId, data);
}
