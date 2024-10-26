import { RoomPersistence } from '../../persistence';
import { RemoveMessagesData, RoomEvent } from './interfaces';

export function removeMessages({ name, amount }: RemoveMessagesData, { roomEvent: { roomId } }: RoomEvent): void {
  RoomPersistence.removeMessages(roomId, name, amount);
}
