import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function replayAdded({ matchInfo }: Data.Event_ReplayAdded): void {
  WebClient.instance.response.session.replayAdded(matchInfo);
}
