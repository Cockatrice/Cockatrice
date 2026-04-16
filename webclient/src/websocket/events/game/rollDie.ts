import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function rollDie(data: Data.Event_RollDie, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.dieRolled(meta.gameId, meta.playerId, data);
}
