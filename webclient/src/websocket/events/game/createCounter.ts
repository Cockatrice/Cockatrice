import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function createCounter(data: Data.Event_CreateCounter, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.counterCreated(meta.gameId, meta.playerId, data);
}
