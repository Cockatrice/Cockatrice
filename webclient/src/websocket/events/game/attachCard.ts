import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function attachCard(data: Data.Event_AttachCard, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.cardAttached(meta.gameId, meta.playerId, data);
}
