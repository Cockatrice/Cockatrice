import { DrawCardsData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function drawCards(data: DrawCardsData, meta: GameEventMeta): void {
  GamePersistence.cardsDrawn(meta.gameId, meta.playerId, data);
}
