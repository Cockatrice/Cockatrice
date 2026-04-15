import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function createToken(data: Data.Event_CreateToken, meta: Enriched.GameEventMeta): void {
  GamePersistence.tokenCreated(meta.gameId, meta.playerId, data);
}
