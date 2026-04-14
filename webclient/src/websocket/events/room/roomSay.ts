import { Message } from 'types';

import { RoomPersistence } from '../../persistence';
import { RoomSayData, RoomEvent } from './interfaces';

export function roomSay(data: RoomSayData, { roomId }: RoomEvent): void {
  const message: Message = { ...data, timeReceived: Date.now() };
  RoomPersistence.addMessage(roomId, message);
}
