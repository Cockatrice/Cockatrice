import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function reverseTurn(data: Data.Event_ReverseTurn, meta: Enriched.GameEventMeta): void {
  GamePersistence.turnReversed(meta.gameId, data.reversed);
}
