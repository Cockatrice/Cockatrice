import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function notifyUser(payload: Data.Event_NotifyUser): void {
  WebClient.instance.response.session.notifyUser(payload);
}
