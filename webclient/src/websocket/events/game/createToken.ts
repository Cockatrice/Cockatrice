import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function createToken(data: Data.Event_CreateToken, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.tokenCreated(meta.gameId, meta.playerId, data);
}
