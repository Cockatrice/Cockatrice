import type { Event_RevealCards } from 'generated/proto/event_reveal_cards_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function revealCards(data: Event_RevealCards, meta: GameEventMeta): void {
  GamePersistence.cardsRevealed(meta.gameId, meta.playerId, data);
}
