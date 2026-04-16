import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';


export function joinGame(data: Data.Event_Join, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.playerJoined(meta.gameId, data.playerProperties);
}
