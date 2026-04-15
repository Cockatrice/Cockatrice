import { Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function leaveGame(data: { reason: number }, meta: Enriched.GameEventMeta): void {
  GamePersistence.playerLeft(meta.gameId, meta.playerId, data.reason ?? 1);
}
