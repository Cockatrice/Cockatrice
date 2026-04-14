import type { Event_DrawCards } from 'generated/proto/event_draw_cards_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function drawCards(data: Event_DrawCards, meta: GameEventMeta): void {
  GamePersistence.cardsDrawn(meta.gameId, meta.playerId, data);
}
