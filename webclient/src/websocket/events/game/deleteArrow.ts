import type { Event_DeleteArrow } from 'generated/proto/event_delete_arrow_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function deleteArrow(data: Event_DeleteArrow, meta: GameEventMeta): void {
  GamePersistence.arrowDeleted(meta.gameId, meta.playerId, data);
}
