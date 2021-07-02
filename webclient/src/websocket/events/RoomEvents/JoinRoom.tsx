export const JoinRoom = {
  id: ".Event_JoinRoom.ext",
  action: ({ userInfo }, webClient, { roomEvent }) => {
    const { roomId } = roomEvent;
    webClient.persistence.room.userJoined(roomId, userInfo);
  }
};
