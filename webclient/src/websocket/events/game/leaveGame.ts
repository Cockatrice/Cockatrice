import { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function leaveGame(data: { reason: number }, meta: GameEventMeta): void {
  GamePersistence.playerLeft(meta.gameId, meta.playerId, data.reason ?? 1);
}
