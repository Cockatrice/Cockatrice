export const JoinRoom = {
  id: '.Event_JoinRoom.ext',
  action: ({ userInfo }, webClient, { roomEvent }) => {
    const { roomId } = roomEvent;
    console.info('Event_JoinRoom', userInfo, roomId);
    webClient.services.room.userJoined(roomId, userInfo);
  }
};