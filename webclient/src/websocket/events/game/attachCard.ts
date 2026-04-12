import { AttachCardData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function attachCard(data: AttachCardData, meta: GameEventMeta): void {
  GamePersistence.cardAttached(meta.gameId, meta.playerId, data);
}
