import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function playerPropertiesChanged(data: Data.Event_PlayerPropertiesChanged, meta: Enriched.GameEventMeta): void {
  GamePersistence.playerPropertiesChanged(meta.gameId, meta.playerId, data.playerProperties);
}
