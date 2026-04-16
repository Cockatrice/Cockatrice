import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function gameSay(data: Data.Event_GameSay, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.gameSay(meta.gameId, meta.playerId, data.message);
}
