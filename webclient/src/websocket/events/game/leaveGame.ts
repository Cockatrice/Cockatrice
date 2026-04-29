import type { Event_Leave } from '@app/generated';

import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function leaveGame(data: Event_Leave, meta: GameEventMeta): void {
  WebClient.instance.response.game.playerLeft(meta.gameId, meta.playerId, data.reason ?? 1);
}
