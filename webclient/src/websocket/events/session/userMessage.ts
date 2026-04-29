import type { Event_UserMessage } from '@app/generated';
import { WebClient } from '../../WebClient';

export function userMessage(payload: Event_UserMessage): void {
  WebClient.instance.response.session.userMessage(payload);
}
