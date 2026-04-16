import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function joinRoom({ userInfo }: Data.Event_JoinRoom, { roomId }: Data.RoomEvent): void {
  WebClient.instance.response.room.userJoined(roomId, userInfo);
}
