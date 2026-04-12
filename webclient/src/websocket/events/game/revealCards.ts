import { GameEventMeta, RevealCardsData } from 'types';
import { GamePersistence } from '../../persistence';

export function revealCards(data: RevealCardsData, meta: GameEventMeta): void {
  GamePersistence.cardsRevealed(meta.gameId, meta.playerId, data);
}
