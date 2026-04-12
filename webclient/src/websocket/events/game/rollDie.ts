import { GameEventMeta, RollDieData } from 'types';
import { GamePersistence } from '../../persistence';

export function rollDie(data: RollDieData, meta: GameEventMeta): void {
  GamePersistence.dieRolled(meta.gameId, meta.playerId, data);
}
