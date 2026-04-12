import { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function gameClosed(_data: {}, meta: GameEventMeta): void {
  GamePersistence.gameClosed(meta.gameId);
}
