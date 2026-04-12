import { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

/**
 * Event_GameHostChanged carries no payload fields.
 * The new host is identified by GameEvent.player_id (meta.playerId).
 */
export function gameHostChanged(_data: {}, meta: GameEventMeta): void {
  GamePersistence.gameHostChanged(meta.gameId, meta.playerId);
}
