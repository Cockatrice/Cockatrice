import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function removeMessages({ name, amount }: Data.Event_RemoveMessages, { roomId }: Data.RoomEvent): void {
  WebClient.instance.response.room.removeMessages(roomId, name, amount);
}
