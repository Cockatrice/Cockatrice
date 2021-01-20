export const ListGames = {
  id: ".Event_ListGames.ext",
  action: ({ gameList }, webClient, { roomEvent }) => {
    const { roomId } = roomEvent;
    webClient.services.room.updateGames(roomId, gameList);
  }
};