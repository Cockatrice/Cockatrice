import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';
import { sanitizeHtml } from '../../utils';

export function serverMessage({ message }: Data.Event_ServerMessage): void {
  WebClient.instance.response.session.serverMessage(sanitizeHtml(message));
}
