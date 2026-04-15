import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function gameSay(data: Data.Event_GameSay, meta: Enriched.GameEventMeta): void {
  GamePersistence.gameSay(meta.gameId, meta.playerId, data.message);
}
