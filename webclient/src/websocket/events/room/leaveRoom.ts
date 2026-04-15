import type { Data } from '@app/types';
import { RoomPersistence } from '../../persistence';

export function leaveRoom({ name }: Data.Event_LeaveRoom, { roomId }: Data.RoomEvent): void {
  RoomPersistence.userLeft(roomId, name);
}
