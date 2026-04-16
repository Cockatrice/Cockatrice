import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function setCardAttr(data: Data.Event_SetCardAttr, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.cardAttrChanged(meta.gameId, meta.playerId, data);
}
