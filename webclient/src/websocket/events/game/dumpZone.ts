import type { Event_DumpZone } from 'generated/proto/event_dump_zone_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function dumpZone(data: Event_DumpZone, meta: GameEventMeta): void {
  GamePersistence.zoneDumped(meta.gameId, meta.playerId, data);
}
