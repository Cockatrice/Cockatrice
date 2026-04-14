import type { Event_GameStateChanged } from 'generated/proto/event_game_state_changed_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function gameStateChanged(data: Event_GameStateChanged, meta: GameEventMeta): void {
  GamePersistence.gameStateChanged(meta.gameId, data);
}
