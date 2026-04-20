import type { Event_GameClosed } from '@app/generated';

import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function gameClosed(_data: Event_GameClosed, meta: GameEventMeta): void {
  WebClient.instance.response.game.gameClosed(meta.gameId);
}
