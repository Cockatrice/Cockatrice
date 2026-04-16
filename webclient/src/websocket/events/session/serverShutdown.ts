import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function serverShutdown(payload: Data.Event_ServerShutdown): void {
  WebClient.instance.response.session.serverShutdown(payload);
}
