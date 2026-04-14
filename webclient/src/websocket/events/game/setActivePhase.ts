import type { Event_SetActivePhase } from 'generated/proto/event_set_active_phase_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function setActivePhase(data: Event_SetActivePhase, meta: GameEventMeta): void {
  GamePersistence.activePhaseSet(meta.gameId, data.phase);
}
