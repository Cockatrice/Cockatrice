import type { Event_RemoveMessages, RoomEvent } from '@app/generated';
import { WebClient } from '../../WebClient';

export function removeMessages({ name, amount }: Event_RemoveMessages, { roomId }: RoomEvent): void {
  WebClient.instance.response.room.removeMessages(roomId, name, amount);
}
