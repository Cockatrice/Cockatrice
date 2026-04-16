import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function setCounter(data: Data.Event_SetCounter, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.counterSet(meta.gameId, meta.playerId, data);
}
