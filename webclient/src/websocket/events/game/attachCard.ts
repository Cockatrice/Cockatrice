import type { Event_AttachCard } from '@app/generated';
import type { GameEventMeta } from '../../types';
import { WebClient } from '../../WebClient';

export function attachCard(data: Event_AttachCard, meta: GameEventMeta): void {
  WebClient.instance.response.game.cardAttached(meta.gameId, meta.playerId, data);
}
