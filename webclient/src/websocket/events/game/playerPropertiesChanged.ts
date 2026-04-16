import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function playerPropertiesChanged(data: Data.Event_PlayerPropertiesChanged, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.playerPropertiesChanged(meta.gameId, meta.playerId, data.playerProperties);
}
