import { RoomPersistence } from "../../persistence";
import { JoinRoomData, RoomEvent } from "./interfaces";

export function joinRoom({ userInfo }: JoinRoomData, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;

  RoomPersistence.userJoined(roomId, userInfo);
}
