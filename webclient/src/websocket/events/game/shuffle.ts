import type { Event_Shuffle } from 'generated/proto/event_shuffle_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function shuffle(data: Event_Shuffle, meta: GameEventMeta): void {
  GamePersistence.zoneShuffled(meta.gameId, meta.playerId, data);
}
