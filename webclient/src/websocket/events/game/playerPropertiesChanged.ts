import type { Event_PlayerPropertiesChanged } from '@app/generated';
import type { GameEventMeta } from '../../interfaces/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function playerPropertiesChanged(data: Event_PlayerPropertiesChanged, meta: GameEventMeta): void {
  WebClient.instance.response.game.playerPropertiesChanged(meta.gameId, meta.playerId, data.playerProperties);
}
