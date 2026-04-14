import type { Event_SetCardCounter } from 'generated/proto/event_set_card_counter_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function setCardCounter(data: Event_SetCardCounter, meta: GameEventMeta): void {
  GamePersistence.cardCounterChanged(meta.gameId, meta.playerId, data);
}
