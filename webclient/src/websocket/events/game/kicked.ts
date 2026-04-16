import type { GameEventMeta } from '../../types';
import { WebClient } from '../../WebClient';

export function kicked(_data: {}, meta: GameEventMeta): void {
  WebClient.instance.response.game.kicked(meta.gameId);
}
