import type { Event_ReverseTurn } from 'generated/proto/event_reverse_turn_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function reverseTurn(data: Event_ReverseTurn, meta: GameEventMeta): void {
  GamePersistence.turnReversed(meta.gameId, data.reversed);
}
