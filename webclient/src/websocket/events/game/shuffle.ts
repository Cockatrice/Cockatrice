import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function shuffle(data: Data.Event_Shuffle, meta: Enriched.GameEventMeta): void {
  GamePersistence.zoneShuffled(meta.gameId, meta.playerId, data);
}
