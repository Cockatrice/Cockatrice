import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function deleteArrow(data: Data.Event_DeleteArrow, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.arrowDeleted(meta.gameId, meta.playerId, data);
}
