import type { Event_ChangeZoneProperties } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function changeZoneProperties(data: Event_ChangeZoneProperties, meta: GameEventMeta): void {
  WebClient.instance.response.game.zonePropertiesChanged(meta.gameId, meta.playerId, data);
}
