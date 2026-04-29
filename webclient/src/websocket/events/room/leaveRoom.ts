import type { Event_LeaveRoom, RoomEvent } from '@app/generated';
import { WebClient } from '../../WebClient';

export function leaveRoom({ name }: Event_LeaveRoom, { roomId }: RoomEvent): void {
  WebClient.instance.response.room.userLeft(roomId, name);
}
