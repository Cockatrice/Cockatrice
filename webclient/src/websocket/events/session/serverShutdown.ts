import type { Event_ServerShutdown } from '@app/generated';
import { WebClient } from '../../WebClient';

export function serverShutdown(payload: Event_ServerShutdown): void {
  WebClient.instance.response.session.serverShutdown(payload);
}
