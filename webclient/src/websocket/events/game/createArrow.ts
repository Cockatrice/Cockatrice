import type { Event_CreateArrow } from '@app/generated';
import type { GameEventMeta } from '../../types';
import { WebClient } from '../../WebClient';

export function createArrow(data: Event_CreateArrow, meta: GameEventMeta): void {
  WebClient.instance.response.game.arrowCreated(meta.gameId, meta.playerId, data);
}
