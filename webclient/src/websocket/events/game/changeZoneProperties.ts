import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function changeZoneProperties(data: Data.Event_ChangeZoneProperties, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.zonePropertiesChanged(meta.gameId, meta.playerId, data);
}
