import { Message } from "types";

import { RoomPersistence } from "../../persistence";
import { RoomEvent } from "./interfaces";

export function roomSay(message: Message, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;
  RoomPersistence.addMessage(roomId, message);
}