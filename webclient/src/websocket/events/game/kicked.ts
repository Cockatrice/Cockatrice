import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function kicked(_data: {}, meta: GameEventMeta): void {
  WebClient.instance.response.game.kicked(meta.gameId);
}
