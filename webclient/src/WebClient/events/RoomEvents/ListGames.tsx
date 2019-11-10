export const ListGames = {
  id: '.Event_ListGames.ext',
  action: ({ gameList }, webClient, { roomEvent }) => {
  	const { roomId } = roomEvent;
  	webClient.services.rooms.updateGames(roomId, gameList)
  }
};