import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function gameClosed(_data: {}, meta: GameEventMeta): void {
  WebClient.instance.response.game.gameClosed(meta.gameId);
}
