import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function destroyCard(data: Data.Event_DestroyCard, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.cardDestroyed(meta.gameId, meta.playerId, data);
}
