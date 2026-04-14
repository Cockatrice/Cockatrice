import type { Event_DestroyCard } from 'generated/proto/event_destroy_card_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function destroyCard(data: Event_DestroyCard, meta: GameEventMeta): void {
  GamePersistence.cardDestroyed(meta.gameId, meta.playerId, data);
}
