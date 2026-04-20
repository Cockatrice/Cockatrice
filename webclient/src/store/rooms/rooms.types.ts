import { roomsSlice } from './rooms.reducer';

const a = roomsSlice.actions;

export const Types = {
  CLEAR_STORE: a.clearStore.type,
  UPDATE_ROOMS: a.updateRooms.type,
  JOIN_ROOM: a.joinRoom.type,
  LEAVE_ROOM: a.leaveRoom.type,
  ADD_MESSAGE: a.addMessage.type,
  UPDATE_GAMES: a.updateGames.type,
  USER_JOINED: a.userJoined.type,
  USER_LEFT: a.userLeft.type,
  SORT_GAMES: a.sortGames.type,
  REMOVE_MESSAGES: a.removeMessages.type,
  GAME_CREATED: a.gameCreated.type,
  JOINED_GAME: a.joinedGame.type,
  SELECT_GAME: a.selectGame.type,
  SET_GAME_FILTERS: a.setGameFilters.type,
  CLEAR_GAME_FILTERS: a.clearGameFilters.type,
  SET_JOIN_GAME_PENDING: a.setJoinGamePending.type,
  SET_JOIN_GAME_ERROR: a.setJoinGameError.type,
  CLEAR_JOIN_GAME_ERROR: a.clearJoinGameError.type,
} as const;

export { MAX_ROOM_MESSAGES } from './rooms.reducer';
