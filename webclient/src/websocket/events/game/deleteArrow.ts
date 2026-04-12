import { DeleteArrowData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function deleteArrow(data: DeleteArrowData, meta: GameEventMeta): void {
  GamePersistence.arrowDeleted(meta.gameId, meta.playerId, data);
}
