import type { Event_ReplayAdded } from '@app/generated';
import { WebClient } from '../../WebClient';

export function replayAdded({ matchInfo }: Event_ReplayAdded): void {
  WebClient.instance.response.session.replayAdded(matchInfo);
}
