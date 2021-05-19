export const RoomSay = {
  id: ".Event_RoomSay.ext",
  action: (message, webClient, { roomEvent }) => {
    const { roomId } = roomEvent;
    webClient.persistence.room.addMessage(roomId, message);
  }
};
