export const LeaveRoom = {
  id: '.Event_LeaveRoom.ext',
  action: ({ name }, webClient, { roomEvent }) => {
    const { roomId } = roomEvent;
    console.info('Event_LeaveRoom', name, roomId);
    webClient.services.room.userLeft(roomId, name);
  }
};