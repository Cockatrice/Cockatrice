import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function dumpZone(data: Data.Event_DumpZone, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.zoneDumped(meta.gameId, meta.playerId, data);
}
