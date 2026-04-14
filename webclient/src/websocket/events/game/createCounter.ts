import type { Event_CreateCounter } from 'generated/proto/event_create_counter_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function createCounter(data: Event_CreateCounter, meta: GameEventMeta): void {
  GamePersistence.counterCreated(meta.gameId, meta.playerId, data);
}
