import type { Event_Join } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';


export function joinGame(data: Event_Join, meta: GameEventMeta): void {
  WebClient.instance.response.game.playerJoined(meta.gameId, data.playerProperties);
}
