import { GamePersistence } from '../../persistence';
import type { Data, Enriched } from '@app/types';

export function joinGame(data: { playerProperties: Data.ServerInfo_PlayerProperties }, meta: Enriched.GameEventMeta): void {
  GamePersistence.playerJoined(meta.gameId, data.playerProperties);
}
