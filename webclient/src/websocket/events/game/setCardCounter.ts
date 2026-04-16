import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function setCardCounter(data: Data.Event_SetCardCounter, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.cardCounterChanged(meta.gameId, meta.playerId, data);
}
