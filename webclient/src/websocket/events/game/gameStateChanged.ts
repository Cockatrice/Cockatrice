import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function gameStateChanged(data: Data.Event_GameStateChanged, meta: Enriched.GameEventMeta): void {
  GamePersistence.gameStateChanged(meta.gameId, data);
}
