import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function shuffle(data: Data.Event_Shuffle, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.zoneShuffled(meta.gameId, meta.playerId, data);
}
