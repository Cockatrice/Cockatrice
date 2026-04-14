import type { Event_RollDie } from 'generated/proto/event_roll_die_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function rollDie(data: Event_RollDie, meta: GameEventMeta): void {
  GamePersistence.dieRolled(meta.gameId, meta.playerId, data);
}
