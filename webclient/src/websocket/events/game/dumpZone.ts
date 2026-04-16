import type { Event_DumpZone } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function dumpZone(data: Event_DumpZone, meta: GameEventMeta): void {
  WebClient.instance.response.game.zoneDumped(meta.gameId, meta.playerId, data);
}
