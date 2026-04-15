import { GamePersistence } from '../../persistence';
import type { Data, Enriched } from '@app/types';

export function joinGame(data: Data.Event_Join, meta: Enriched.GameEventMeta): void {
  GamePersistence.playerJoined(meta.gameId, data.playerProperties);
}
