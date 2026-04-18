import type { Event_GameStateChanged } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function gameStateChanged(data: Event_GameStateChanged, meta: GameEventMeta): void {
  WebClient.instance.response.game.gameStateChanged(meta.gameId, data);
}
