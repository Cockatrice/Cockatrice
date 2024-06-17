import { RoomPersistence } from '../../persistence';
import { LeaveRoomData, RoomEvent } from './interfaces';

export function leaveRoom({ name }: LeaveRoomData, { roomEvent: { roomId } }: RoomEvent): void {
  RoomPersistence.userLeft(roomId, name);
}
