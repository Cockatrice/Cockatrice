import type { Event_SetCounter } from 'generated/proto/event_set_counter_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function setCounter(data: Event_SetCounter, meta: GameEventMeta): void {
  GamePersistence.counterSet(meta.gameId, meta.playerId, data);
}
