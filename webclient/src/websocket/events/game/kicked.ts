import { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function kicked(_data: {}, meta: GameEventMeta): void {
  GamePersistence.kicked(meta.gameId);
}
