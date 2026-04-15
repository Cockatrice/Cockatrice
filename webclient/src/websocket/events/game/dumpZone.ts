import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function dumpZone(data: Data.Event_DumpZone, meta: Enriched.GameEventMeta): void {
  GamePersistence.zoneDumped(meta.gameId, meta.playerId, data);
}
