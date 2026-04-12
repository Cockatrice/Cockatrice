import { GameEventMeta, MoveCardData } from 'types';
import { GamePersistence } from '../../persistence';

export function moveCard(data: MoveCardData, meta: GameEventMeta): void {
  GamePersistence.cardMoved(meta.gameId, meta.playerId, data);
}
