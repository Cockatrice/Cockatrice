import { CreateArrowData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function createArrow(data: CreateArrowData, meta: GameEventMeta): void {
  GamePersistence.arrowCreated(meta.gameId, meta.playerId, data);
}
