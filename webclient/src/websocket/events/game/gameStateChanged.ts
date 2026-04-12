import { GameEventMeta, GameStateChangedData } from 'types';
import { GamePersistence } from '../../persistence';

export function gameStateChanged(data: GameStateChangedData, meta: GameEventMeta): void {
  GamePersistence.gameStateChanged(meta.gameId, data);
}
