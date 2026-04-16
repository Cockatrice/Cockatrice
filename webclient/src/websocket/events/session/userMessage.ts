import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function userMessage(payload: Data.Event_UserMessage): void {
  WebClient.instance.response.session.userMessage(payload);
}
