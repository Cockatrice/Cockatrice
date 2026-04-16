import type { Event_RollDie } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function rollDie(data: Event_RollDie, meta: GameEventMeta): void {
  WebClient.instance.response.game.dieRolled(meta.gameId, meta.playerId, data);
}
