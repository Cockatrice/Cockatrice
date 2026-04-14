import type { Event_ChangeZoneProperties } from 'generated/proto/event_change_zone_properties_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function changeZoneProperties(data: Event_ChangeZoneProperties, meta: GameEventMeta): void {
  GamePersistence.zonePropertiesChanged(meta.gameId, meta.playerId, data);
}
