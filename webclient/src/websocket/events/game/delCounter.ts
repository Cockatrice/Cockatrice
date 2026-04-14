import type { Event_DelCounter } from 'generated/proto/event_del_counter_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function delCounter(data: Event_DelCounter, meta: GameEventMeta): void {
  GamePersistence.counterDeleted(meta.gameId, meta.playerId, data);
}
