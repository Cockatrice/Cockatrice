import type { Event_MoveCard } from 'generated/proto/event_move_card_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function moveCard(data: Event_MoveCard, meta: GameEventMeta): void {
  GamePersistence.cardMoved(meta.gameId, meta.playerId, data);
}
