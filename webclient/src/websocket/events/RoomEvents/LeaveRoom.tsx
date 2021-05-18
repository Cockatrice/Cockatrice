export const LeaveRoom = {
  id: ".Event_LeaveRoom.ext",
  action: ({ name }, webClient, { roomEvent }) => {
    const { roomId } = roomEvent;
    webClient.persistence.room.userLeft(roomId, name);
  }
};
