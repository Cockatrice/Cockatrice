import type { Event_ServerMessage } from '@app/generated';
import { WebClient } from '../../WebClient';
import { sanitizeHtml } from '../../utils';

export function serverMessage({ message }: Event_ServerMessage): void {
  WebClient.instance.response.session.serverMessage(sanitizeHtml(message));
}
