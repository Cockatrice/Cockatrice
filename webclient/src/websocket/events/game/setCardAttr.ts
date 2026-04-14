import type { Event_SetCardAttr } from 'generated/proto/event_set_card_attr_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function setCardAttr(data: Event_SetCardAttr, meta: GameEventMeta): void {
  GamePersistence.cardAttrChanged(meta.gameId, meta.playerId, data);
}
