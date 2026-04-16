import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function reverseTurn(data: Data.Event_ReverseTurn, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.turnReversed(meta.gameId, data.reversed);
}
