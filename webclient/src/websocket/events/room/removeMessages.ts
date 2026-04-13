import { RoomPersistence } from '../../persistence';
import { RemoveMessagesData, RoomEvent } from './interfaces';

export function removeMessages({ name, amount }: RemoveMessagesData, { roomId }: RoomEvent): void {
  RoomPersistence.removeMessages(roomId, name, amount);
}
