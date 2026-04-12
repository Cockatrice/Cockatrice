import { DestroyCardData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function destroyCard(data: DestroyCardData, meta: GameEventMeta): void {
  GamePersistence.cardDestroyed(meta.gameId, meta.playerId, data);
}
