import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function gameStateChanged(data: Data.Event_GameStateChanged, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.gameStateChanged(meta.gameId, data);
}
