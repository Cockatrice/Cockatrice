import type { Event_CreateArrow } from 'generated/proto/event_create_arrow_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function createArrow(data: Event_CreateArrow, meta: GameEventMeta): void {
  GamePersistence.arrowCreated(meta.gameId, meta.playerId, data);
}
