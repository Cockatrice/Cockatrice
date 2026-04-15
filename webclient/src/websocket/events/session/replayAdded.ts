import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function replayAdded({ matchInfo }: Data.Event_ReplayAdded): void {
  SessionPersistence.replayAdded(matchInfo);
}
