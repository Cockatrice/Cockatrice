import type { GameEventMeta } from '../../types';
import { WebClient } from '../../WebClient';

export function leaveGame(data: { reason: number }, meta: GameEventMeta): void {
  WebClient.instance.response.game.playerLeft(meta.gameId, meta.playerId, data.reason ?? 1);
}
