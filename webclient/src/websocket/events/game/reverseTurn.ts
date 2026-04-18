import type { Event_ReverseTurn } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function reverseTurn(data: Event_ReverseTurn, meta: GameEventMeta): void {
  WebClient.instance.response.game.turnReversed(meta.gameId, data.reversed);
}
