import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function userJoined({ userInfo }: Data.Event_UserJoined): void {
  WebClient.instance.response.session.userJoined(userInfo);
}
