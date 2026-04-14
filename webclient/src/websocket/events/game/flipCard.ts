import type { Event_FlipCard } from 'generated/proto/event_flip_card_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function flipCard(data: Event_FlipCard, meta: GameEventMeta): void {
  GamePersistence.cardFlipped(meta.gameId, meta.playerId, data);
}
