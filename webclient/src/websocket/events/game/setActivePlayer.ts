import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function setActivePlayer(data: Data.Event_SetActivePlayer, meta: Enriched.GameEventMeta): void {
  GamePersistence.activePlayerSet(meta.gameId, data.activePlayerId);
}
