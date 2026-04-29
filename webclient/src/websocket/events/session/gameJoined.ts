import type { Event_GameJoined } from '@app/generated';
import { WebClient } from '../../WebClient';

export function gameJoined(gameJoined: Event_GameJoined): void {
  WebClient.instance.response.session.gameJoined(gameJoined);
}
