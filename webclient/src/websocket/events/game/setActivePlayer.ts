import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function setActivePlayer(data: Data.Event_SetActivePlayer, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.activePlayerSet(meta.gameId, data.activePlayerId);
}
