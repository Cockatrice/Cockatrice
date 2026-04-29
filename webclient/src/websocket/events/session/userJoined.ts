import type { Event_UserJoined } from '@app/generated';
import { WebClient } from '../../WebClient';

export function userJoined({ userInfo }: Event_UserJoined): void {
  WebClient.instance.response.session.userJoined(userInfo);
}
