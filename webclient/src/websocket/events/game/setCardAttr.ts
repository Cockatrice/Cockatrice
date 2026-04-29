import type { Event_SetCardAttr } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function setCardAttr(data: Event_SetCardAttr, meta: GameEventMeta): void {
  WebClient.instance.response.game.cardAttrChanged(meta.gameId, meta.playerId, data);
}
