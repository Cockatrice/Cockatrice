import { Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

/**
 * Event_GameHostChanged carries no payload fields.
 * The new host is identified by GameEvent.player_id (meta.playerId).
 */
export function gameHostChanged(_data: {}, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.gameHostChanged(meta.gameId, meta.playerId);
}
