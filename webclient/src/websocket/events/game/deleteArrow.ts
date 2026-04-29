import type { Event_DeleteArrow } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function deleteArrow(data: Event_DeleteArrow, meta: GameEventMeta): void {
  WebClient.instance.response.game.arrowDeleted(meta.gameId, meta.playerId, data);
}
