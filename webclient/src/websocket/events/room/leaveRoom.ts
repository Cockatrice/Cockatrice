import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function leaveRoom({ name }: Data.Event_LeaveRoom, { roomId }: Data.RoomEvent): void {
  WebClient.instance.response.room.userLeft(roomId, name);
}
