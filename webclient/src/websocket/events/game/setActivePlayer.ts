import type { Event_SetActivePlayer } from 'generated/proto/event_set_active_player_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function setActivePlayer(data: Event_SetActivePlayer, meta: GameEventMeta): void {
  GamePersistence.activePlayerSet(meta.gameId, data.activePlayerId);
}
