import { SessionPersistence } from '../../persistence';
import { ReplayAddedData } from './interfaces';

export function replayAdded({ matchInfo }: ReplayAddedData): void {
  SessionPersistence.replayAdded(matchInfo);
}
