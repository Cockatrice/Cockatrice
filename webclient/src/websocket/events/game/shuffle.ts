import { GameEventMeta, ShuffleData } from 'types';
import { GamePersistence } from '../../persistence';

export function shuffle(data: ShuffleData, meta: GameEventMeta): void {
  GamePersistence.zoneShuffled(meta.gameId, meta.playerId, data);
}
