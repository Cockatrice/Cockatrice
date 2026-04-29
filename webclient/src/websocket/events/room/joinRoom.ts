import type { Event_JoinRoom, RoomEvent } from '@app/generated';
import { WebClient } from '../../WebClient';

export function joinRoom({ userInfo }: Event_JoinRoom, { roomId }: RoomEvent): void {
  WebClient.instance.response.room.userJoined(roomId, userInfo);
}
