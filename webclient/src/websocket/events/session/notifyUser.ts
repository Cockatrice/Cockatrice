import type { Event_NotifyUser } from '@app/generated';
import { WebClient } from '../../WebClient';

export function notifyUser(payload: Event_NotifyUser): void {
  WebClient.instance.response.session.notifyUser(payload);
}
