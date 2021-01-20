export const LeaveRoom = {
  id: ".Event_LeaveRoom.ext",
  action: ({ name }, webClient, { roomEvent }) => {
    const { roomId } = roomEvent;
    webClient.services.room.userLeft(roomId, name);
  }
};