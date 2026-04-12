import { FlipCardData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function flipCard(data: FlipCardData, meta: GameEventMeta): void {
  GamePersistence.cardFlipped(meta.gameId, meta.playerId, data);
}
