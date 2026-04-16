import type { Data, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function setActivePhase(data: Data.Event_SetActivePhase, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.activePhaseSet(meta.gameId, data.phase);
}
