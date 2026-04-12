import { DumpZoneData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function dumpZone(data: DumpZoneData, meta: GameEventMeta): void {
  GamePersistence.zoneDumped(meta.gameId, meta.playerId, data);
}
