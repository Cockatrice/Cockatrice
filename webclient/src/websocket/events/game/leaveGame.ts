import { Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function leaveGame(data: { reason: number }, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.playerLeft(meta.gameId, meta.playerId, data.reason ?? 1);
}
