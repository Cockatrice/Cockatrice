import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function moveCard(data: Data.Event_MoveCard, meta: Enriched.GameEventMeta): void {
  GamePersistence.cardMoved(meta.gameId, meta.playerId, data);
}
