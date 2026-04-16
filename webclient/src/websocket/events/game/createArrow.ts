import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function createArrow(data: Data.Event_CreateArrow, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.arrowCreated(meta.gameId, meta.playerId, data);
}
