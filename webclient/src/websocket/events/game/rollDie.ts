import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function rollDie(data: Data.Event_RollDie, meta: Enriched.GameEventMeta): void {
  GamePersistence.dieRolled(meta.gameId, meta.playerId, data);
}
