import { RoomPersistence } from '../../persistence';
import { JoinRoomData, RoomEvent } from './interfaces';

export function joinRoom({ userInfo }: JoinRoomData, { roomEvent: { roomId } }: RoomEvent): void {
  RoomPersistence.userJoined(roomId, userInfo);
}
