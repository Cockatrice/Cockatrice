import type { Event_AttachCard } from 'generated/proto/event_attach_card_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function attachCard(data: Event_AttachCard, meta: GameEventMeta): void {
  GamePersistence.cardAttached(meta.gameId, meta.playerId, data);
}
