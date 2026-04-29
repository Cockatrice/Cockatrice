import type { Event_CreateToken } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function createToken(data: Event_CreateToken, meta: GameEventMeta): void {
  WebClient.instance.response.game.tokenCreated(meta.gameId, meta.playerId, data);
}
