import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function setCardAttr(data: Data.Event_SetCardAttr, meta: Enriched.GameEventMeta): void {
  GamePersistence.cardAttrChanged(meta.gameId, meta.playerId, data);
}
