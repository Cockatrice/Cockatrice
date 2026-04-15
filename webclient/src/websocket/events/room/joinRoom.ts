import type { Data } from '@app/types';
import { RoomPersistence } from '../../persistence';

export function joinRoom({ userInfo }: Data.Event_JoinRoom, { roomId }: Data.RoomEvent): void {
  RoomPersistence.userJoined(roomId, userInfo);
}
