import { Message } from 'types';

import { RoomPersistence } from '../../persistence';
import { RoomEvent } from './interfaces';

export function roomSay(message: Message, { roomId }: RoomEvent): void {
  RoomPersistence.addMessage(roomId, message);
}
