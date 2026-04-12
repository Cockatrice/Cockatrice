import { GameEventMeta, GameSayData } from 'types';
import { GamePersistence } from '../../persistence';

export function gameSay(data: GameSayData, meta: GameEventMeta): void {
  GamePersistence.gameSay(meta.gameId, meta.playerId, data.message);
}
