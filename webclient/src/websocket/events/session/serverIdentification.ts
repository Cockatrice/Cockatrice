import type { Event_ServerIdentification } from '@app/generated';
import { WebClient } from '../../WebClient';

export function serverIdentification(info: Event_ServerIdentification): void {
  WebClient.instance.config.onServerIdentified(info);
}
