import type { Event_GameSay } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function gameSay(data: Event_GameSay, meta: GameEventMeta): void {
  WebClient.instance.response.game.gameSay(meta.gameId, meta.playerId, data.message, Date.now());
}
