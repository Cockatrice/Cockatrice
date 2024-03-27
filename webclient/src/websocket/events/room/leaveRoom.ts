import { RoomPersistence } from "../../persistence";
import { LeaveRoomData, RoomEvent } from "./interfaces";

export function leaveRoom({ name }: LeaveRoomData, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;
  RoomPersistence.userLeft(roomId, name);
}
