import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function gameClosed(_data: {}, meta: GameEventMeta): void {
  WebClient.instance.response.game.gameClosed(meta.gameId);
}
