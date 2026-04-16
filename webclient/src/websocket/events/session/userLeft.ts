import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function userLeft({ name }: Data.Event_UserLeft): void {
  WebClient.instance.response.session.userLeft(name);
}
