import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function playerPropertiesChanged(data: { playerProperties: Data.ServerInfo_PlayerProperties }, meta: Enriched.GameEventMeta): void {
  GamePersistence.playerPropertiesChanged(meta.gameId, meta.playerId, data.playerProperties);
}
