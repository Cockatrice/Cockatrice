import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function delCounter(data: Data.Event_DelCounter, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.counterDeleted(meta.gameId, meta.playerId, data);
}
