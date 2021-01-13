export const RoomSay = {
  id: ".Event_RoomSay.ext",
  action: (message, webClient, { roomEvent }) => {
    const { roomId } = roomEvent;
    webClient.services.room.addMessage(roomId, message);
  }
};