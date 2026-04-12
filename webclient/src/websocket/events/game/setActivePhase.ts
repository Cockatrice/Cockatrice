import { GameEventMeta, SetActivePhaseData } from 'types';
import { GamePersistence } from '../../persistence';

export function setActivePhase(data: SetActivePhaseData, meta: GameEventMeta): void {
  GamePersistence.activePhaseSet(meta.gameId, data.phase);
}
