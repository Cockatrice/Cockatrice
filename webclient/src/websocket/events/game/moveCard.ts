import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function moveCard(data: Data.Event_MoveCard, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.cardMoved(meta.gameId, meta.playerId, data);
}
