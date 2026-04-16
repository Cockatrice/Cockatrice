import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function gameJoined(gameJoined: Data.Event_GameJoined): void {
  WebClient.instance.response.session.gameJoined(gameJoined);
}
