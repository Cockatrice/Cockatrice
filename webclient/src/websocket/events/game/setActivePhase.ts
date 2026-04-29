import type { Event_SetActivePhase } from '@app/generated';
import type { GameEventMeta } from '../../types/WebSocketConfig';
import { WebClient } from '../../WebClient';

export function setActivePhase(data: Event_SetActivePhase, meta: GameEventMeta): void {
  WebClient.instance.response.game.activePhaseSet(meta.gameId, data.phase);
}
