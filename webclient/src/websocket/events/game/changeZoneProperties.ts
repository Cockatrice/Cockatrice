import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function changeZoneProperties(data: Data.Event_ChangeZoneProperties, meta: Enriched.GameEventMeta): void {
  GamePersistence.zonePropertiesChanged(meta.gameId, meta.playerId, data);
}
