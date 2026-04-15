import type { Data, Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function setActivePhase(data: Data.Event_SetActivePhase, meta: Enriched.GameEventMeta): void {
  GamePersistence.activePhaseSet(meta.gameId, data.phase);
}
