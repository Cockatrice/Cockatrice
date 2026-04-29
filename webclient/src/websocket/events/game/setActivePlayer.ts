import type { Event_SetActivePlayer } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function setActivePlayer(data: Event_SetActivePlayer, meta: GameEventMeta): void {
  WebClient.instance.response.game.activePlayerSet(meta.gameId, data.activePlayerId);
}
